using System.Xml.Linq;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;
using XRay.Office.Text.Shaping;
using XRay.Office.WordProcessing.Layout;

namespace XRay.Office.WordProcessing.Ooxml;

/// <summary>
/// The list half of the DOCX layout walk: which list instance and level a paragraph names, what its
/// label says, and where <c>numbering.xml</c> puts both.
/// </summary>
/// <remarks>
/// Unlike ODF, nesting says nothing: a paragraph states its own <c>w:ilvl</c> and <c>w:numId</c>, so the
/// only thing the walk has to carry is the counters — which it must, because the label is nowhere in the
/// file and a level's value is "how many items at this level have been passed".
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>The document's numbering, or an empty one when it declares none.</summary>
    private readonly WordNumbering _numbering;

    /// <summary>
    /// The label and the indents a paragraph's list level gives it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>Advancing a counter is a side effect, so this may be called once per paragraph.</strong>
    /// It is called from <see cref="Paragraph"/> in document order, which is the order the counters have
    /// to see — the same reason the extraction reader advances them where it does.
    /// </para>
    /// <para>
    /// Whether the level's own <c>w:pPr/w:ind</c> applies is Writer's
    /// <c>SwTextNode::AreListLevelIndentsApplicable</c> — see
    /// <see cref="WordParagraphFormats.ListLevelIndentsApplicable"/>, which ports it. LibreOffice's own
    /// DOCX export writes the level's indent onto every list paragraph as well as into
    /// <c>numbering.xml</c>, so on those documents the two agree and this only matters for files that
    /// disagree — which Word's own output does, since <c>ListParagraph</c> carries a left indent and no
    /// hanging one.
    /// </para>
    /// </remarks>
    /// <param name="properties">The paragraph's <c>w:pPr</c>, or null.</param>
    /// <param name="format">Its resolved formatting.</param>
    /// <param name="text">Its resolved text style, for the size and colour the label inherits.</param>
    /// <param name="face">Its face, which the label falls back to when the level names none.</param>
    private (PageLabel? Label, ParagraphFormat Format) ListFormatting(
        XElement? properties, ParagraphFormat format, WordTextStyle text, OpenTypeFace face)
    {
        if (ResolveNumbering(properties) is not (string numId, int level)) return (null, format);
        if (_numbering.FindLevel(numId, level) is not { } definition) return (null, format);

        // Advanced whether or not the level draws anything, so that a level formatted `none` still
        // counts — a numbered heading that shows no number is still the third heading.
        //
        // The exception is a paragraph in a cell covered by a vertical merge: it is not drawn at all,
        // and LibreOffice clears its counted-in-list flag for exactly that reason
        // (`sw/source/core/unocore/unotbl.cxx`:978-990). Not counted, so not advanced and not labelled;
        // `drawn` stays null and the branch below returns the paragraph's own first-line indent.
        string? drawn = _inCoveredCell ? null : _numbering.Advance(numId, level);

        XElement? levelProperties = definition.ParagraphProperties;
        XElement? indent = Word.Child(levelProperties, "ind");
        Length ownFirstLine = format.FirstLineIndent;

        if (indent is not null)
        {
            ListLevelIndents applicable =
                WordParagraphFormats.ListLevelIndentsApplicable(_styles, properties);

            if (applicable.HasFlag(ListLevelIndents.LeftMargin))
            {
                format = format with
                {
                    StartIndent =
                        Twips(indent, "start") ?? Twips(indent, "left") ?? format.StartIndent,
                };
            }

            if (applicable.HasFlag(ListLevelIndents.FirstLine))
            {
                format = format with
                {
                    FirstLineIndent = Twips(indent, "hanging") is { } hanging
                        ? -hanging
                        : Twips(indent, "firstLine") ?? format.FirstLineIndent,
                };
            }
        }

        LabelFollow follow = definition.Suffix switch
        {
            "space" => LabelFollow.Space,
            "nothing" => LabelFollow.Nothing,
            _ => LabelFollow.ListTab,
        };

        // The stop the label's tab aims at is the level's own `w:tab w:val="num"`, which LibreOffice
        // reads into the list's tab position rather than into the paragraph's stops
        // (`SwTextNode::GetListTabStopPosition`). Falling back to the text indent is what a level that
        // states no stop means: the tab lands where the paragraph's text starts.
        Length tabStop = NumberingTab(levelProperties) ?? format.StartIndent;

        if (drawn is not { Length: > 0 })
        {
            // No label, so no hanging: `SwTextNode::GetFirstLineOfsWithNum` applies the numbering's
            // offset only to a node that is counted in its list.
            return (null, format with { FirstLineIndent = ownFirstLine });
        }

        (string labelText, OpenTypeFace labelFace, FontReference? labelFont) =
            LabelFace(
                definition, text, face, drawn,
                Stated(definition.RunProperties),
                Stated(Word.Child(properties, "rPr")));

        PageLabel label = PageLabel.Measured(
            labelText, labelFace, LabelSize(definition, text),
            new ShapingOptions(Language: text.Language, DisableKerning: !text.AutoKerning));

        return (
            label with
            {
                Font = labelFont,
                Colour = text.Colour ?? Core.Graphics.Colour.Transparent,
                Follow = follow,
                TabStop = tabStop,
            },
            format);
    }

    /// <summary>
    /// The list instance and level a paragraph belongs to, or null when it is not in one.
    /// </summary>
    /// <remarks>
    /// The reference may be direct on the paragraph, inherited from its style, or implied by a
    /// style-linked list that names the style rather than the other way round — the last is how numbered
    /// headings are usually written. A <c>w:numId</c> of zero means "not numbered", which is how Word
    /// writes a continuation paragraph inside a list item, and must not be read as the first list.
    /// <para>
    /// <strong><c>w:numPr</c> is a bag of properties and not one value, so <c>w:numId</c> and
    /// <c>w:ilvl</c> inherit separately.</strong> This is <see cref="WordStyles.ParagraphPropertyLayers"/>'s
    /// rule, already stated there for <c>w:spacing</c> and <c>w:ind</c>, and <c>w:numPr</c> is the third
    /// element it applies to: Word maps the two children to separate properties, so the nearest layer
    /// stating each one wins independently.
    /// </para>
    /// <para>
    /// Taking the innermost <c>w:numPr</c> whole is how an outline-numbered heading loses its number
    /// entirely, which is the ordinary shape of a Word heading chain rather than an exotic one. Measured on
    /// <c>report-template.docx</c>: <c>Heading2</c> states <c>&lt;w:numPr&gt;&lt;w:numId w:val="9"/&gt;</c>
    /// with no level, <c>Heading3</c> and <c>Heading4</c> each restate only <c>&lt;w:ilvl&gt;</c>, and a
    /// paragraph styled <c>Heading4</c> carries no <c>w:numPr</c> of its own. Stopping at the first
    /// <c>w:numPr</c> in the chain finds <c>Heading4</c>'s, reads no <c>w:numId</c> from it and answers
    /// "not numbered" — so every heading from level 3 down drew unnumbered where the reference draws
    /// <c>1.1.1</c>.
    /// </para>
    /// </remarks>
    private (string NumId, int Level)? ResolveNumbering(XElement? properties)
    {
        string? styleId = Word.Attribute(Word.Child(properties, "pStyle"), "val");

        string? numId = null;
        int? level = null;

        foreach (XElement layer in _styles.ParagraphPropertyLayers("numPr", properties, styleId))
        {
            numId ??= Word.Value(layer, "numId");

            if (level is null && int.TryParse(Word.Value(layer, "ilvl"), out int stated))
            {
                level = stated;
            }

            if (numId is not null && level is not null) break;
        }

        if (numId is null && styleId is not null
            && _numbering.FindInstanceForStyle(styleId) is { } styleLinked)
        {
            numId = styleLinked;
        }

        if (numId is null or "0") return null;
        return (numId, Math.Clamp(level ?? 0, 0, WordNumbering.LevelCount - 1));
    }

    /// <summary>
    /// What the label draws, and the face it is drawn in.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A bullet level names a symbol font and states its bullet as a code point in that font's private
    /// use area, which means nothing anywhere else. <see cref="WordNumbering.FormatLabel"/> has turned
    /// such a code point into U+2022, which is the right answer only when nothing better is available —
    /// LibreOffice recodes the slot into OpenSymbol and draws the picture the document asked for. See
    /// <see cref="Symbol(string?, char, bool)"/>, which decides between the cases; U+2022 in the paragraph's own face is
    /// what is left when it declines.
    /// </para>
    /// <para>
    /// A level naming a font for an ordinary character keeps it.
    /// </para>
    /// </remarks>
    /// <returns>
    /// The text to draw, the face, and the reference it was resolved through. The reference because it
    /// cannot be recovered from the face afterwards: an <see cref="OpenTypeFace"/> is a parsed table
    /// directory and does not carry the path it was read from, and the path is the only thing a
    /// PDF can turn back into an embedded font program. A label drawn from a family-named
    /// reference is referenced and not embedded, which is what <c>pdffonts</c> reported for
    /// <c>word-features.docx</c>'s <c>LiberationSerif</c> and <c>OpenSymbol</c> labels while every
    /// body face in the same file embedded.
    /// </returns>
    private (string Text, OpenTypeFace Face, FontReference? Font) LabelFace(
        WordNumberingLevel definition,
        WordTextStyle text,
        OpenTypeFace face,
        string drawn,
        bool? levelItalic,
        bool? markItalic)
    {
        FontReference? own = _references.GetValueOrDefault(text.FaceKey);

        XElement? levelFonts = Word.Child(definition.RunProperties, "rFonts");

        string? family = WordParagraphFormats.SlotFamily(
            levelFonts, _theme?.Fonts, "ascii", "asciiTheme");

        // The level's own `w:rFonts` is a property layer like any other, so it states a class only
        // when its `w:ascii` names a font the table files; otherwise the paragraph's stands. See
        // `WordParagraphFormats.StatedClass`.
        FontFamilyClass levelClass =
            WordParagraphFormats.StatedClass(levelFonts is { } stated ? [stated] : [], _fontTable)
                is var declared and not FontFamilyClass.Unknown
                ? declared
                : text.DeclaredClass;

        if (definition.LevelText is [>= '\uE000' and <= '\uF8FF' and var slot])
        {
            // A bullet's base font is the paragraph's with its posture and weight reset — #i53199 in
            // `SwTextFormatter::NewNumberPortion`, `sw/source/core/text/txtfld.cxx`:578-590 — so the
            // paragraph's *style* cannot reach it and only the level's own character formatting and
            // the paragraph mark's direct formatting can. Measured, `probes/words-r59/label-slant.py`:
            // a `w:i` on the style leans the item's text and leaves the bullet upright, where the same
            // `w:i` written directly on the paragraph's `w:pPr/w:rPr` leans both.
            bool italic = levelItalic ?? markItalic ?? false;

            return Symbol(family, slot, italic)
                   ?? Restyled(text, face, own, italic, drawn);
        }

        // A number label keeps the paragraph's own posture — the number branch of the same function
        // resets only underline and overline — so the base is the mark's resolved italic and the
        // level overrides it only where it states one. An explicit `<w:i w:val="0"/>` on the level
        // therefore draws the number upright over an italic paragraph, which 13 of the corpus's
        // `.docx` state and two of them draw.
        bool numberItalic = levelItalic ?? text.IsItalic;

        if (family is not { Length: > 0 }) return Restyled(text, face, own, numberItalic, drawn);

        WordTextStyle named = text with
        {
            FamilyName = family,
            DeclaredClass = levelClass,
            IsItalic = numberItalic,
        };

        return Face(named) is { } resolved
            ? (drawn, resolved, _references.GetValueOrDefault(named.FaceKey))
            : Restyled(text, face, own, numberItalic, drawn);
    }

    /// <summary>
    /// The paragraph's own face at the label's posture, or the paragraph's face unchanged when the
    /// two agree or when the restyled request resolves to nothing.
    /// </summary>
    /// <remarks>
    /// The caller's fallback is the paragraph's face, and re-resolving it unconditionally would put a
    /// second request through <see cref="Face"/> for every list paragraph in the corpus to get the
    /// answer it already had. So the request is made only where the label's posture differs from the
    /// paragraph's, which is the only case that can change what is drawn.
    /// </remarks>
    private (string Text, OpenTypeFace Face, FontReference? Font) Restyled(
        WordTextStyle text, OpenTypeFace face, FontReference? own, bool italic, string drawn)
    {
        if (italic == text.IsItalic) return (drawn, face, own);

        WordTextStyle leaned = text with { IsItalic = italic };

        return Face(leaned) is { } resolved
            ? (drawn, resolved, _references.GetValueOrDefault(leaned.FaceKey))
            : (drawn, face, own);
    }

    /// <summary>
    /// Whether a <c>w:rPr</c> states a posture, and which — null when it says nothing at all.
    /// </summary>
    /// <remarks>
    /// Three states rather than two, because *stated off* is not the same as *unstated* for a list
    /// label: the level's own <c>&lt;w:i w:val="0"/&gt;</c> beats an italic paragraph mark and an
    /// absent <c>w:i</c> lets the mark through. Measured on `label-slant.py`'s `leveloff-markon` and
    /// `levelon-markoff`, which differ by exactly that and disagree on both label kinds.
    /// </remarks>
    private static bool? Stated(XElement? runProperties)
        => Word.Child(runProperties, "i") is { } posture ? Word.IsOn(posture) : null;

    /// <summary>
    /// A symbol level's slot, drawn from the face that can actually show it, or null when none can.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>The trigger is that the face itself is absent, not that the request happened to resolve
    /// to OpenSymbol.</strong> When the face is installed, the slot is drawn from it unchanged. When it
    /// is not — and Wingdings, Webdings and Monotype Sorts are not fonts Linux has — LibreOffice
    /// substitutes OpenSymbol and recodes through the tables <see cref="SymbolFontRecode"/> ports, whose
    /// F000–F0FF coverage is ten code points, so drawing the slot there instead would be
    /// <c>.notdef</c>.
    /// </para>
    /// <para>
    /// Keying on the <em>resolved</em> family is too narrow: it works for the faces <c>VCL.xcu</c>
    /// gives a substitution chain — Wingdings names <c>opensymbol</c> fourth — and silently fails for
    /// the ones it does not, since nothing in that table mentions <c>monotypesorts</c> or
    /// <c>mtextra</c>. LibreOffice never asks fontconfig about a symbol font at all
    /// (<c>FcPreMatchSubstitution::FindFontSubstitute</c> returns false outright for one,
    /// <c>vcl/unx/generic/font/fontsubst.cxx:100-107</c>), which is why the absence of a chain costs it
    /// nothing. Mirrors <c>SlideTextLayout.Recoded</c>, which reached the same shape from the
    /// presentation side.
    /// </para>
    /// <para>
    /// Null means nothing here can improve on the caller's fallback, which is U+2022 in the
    /// paragraph's own face — a symbol face with no table, a slot out of range, or a resolution that
    /// failed.
    /// </para>
    /// </remarks>
    /// <summary>
    /// The two-argument form the run walker binds to, for a <c>w:sym</c> inside ordinary text.
    /// </summary>
    /// <remarks>
    /// Upright, because the walker is given a family and a slot and not the run's <c>w:rPr</c>.
    /// Whether 26.2.4.2 leans a recoded <c>w:sym</c> in an italic run is <strong>not
    /// measured</strong> — round 58's residual is a column of list bullets at one x position and
    /// holds no run symbol at all, so no corpus witness separates the two answers. Named here so
    /// that a round which finds one knows where it lives.
    /// </remarks>
    private (string Text, OpenTypeFace Face, FontReference? Font)? Symbol(string? family, char slot)
        => Symbol(family, slot, italic: false);

    private (string Text, OpenTypeFace Face, FontReference? Font)? Symbol(
        string? family, char slot, bool italic)
    {
        if (family is not { Length: > 0 }) return null;
        if (!SymbolFontRecode.IsRecodeable(family)) return null;

        // The weight is the bullet's reset one and the size is decided by LabelSize and plays no
        // part in which file is loaded. The posture is the level's, and it reaches the page as a
        // synthetic lean rather than as a face: OpenSymbol ships one cut, so
        // `SystemFontResolver.Resolve` answers the upright file with `SyntheticOblique` set — which
        // is the same shear the reference draws, `1 0 0.3462 1 … Tm` in its own content stream.
        WordTextStyle stated = new(family, Length.Zero, 400, italic, null);
        if (Face(stated) is not { } statedFace) return null;

        FontReference? reference = _references.GetValueOrDefault(stated.FaceKey);

        // The face's own file is present, so its slots are drawable as they stand.
        if (reference is not null
            && !reference.IsSubstituted
            && !SymbolFontRecode.IsSubstituteFamily(reference.FamilyName))
        {
            return (slot.ToString(), statedFace, reference);
        }

        if (!SymbolFontRecode.TryRecode(family, slot, out char recoded)) return null;

        // The recode and the face go together: the code point means nothing anywhere but OpenSymbol,
        // so a resolution that landed elsewhere leaves the caller's fallback in place rather than
        // drawing it out of whatever the request happened to reach.
        WordTextStyle substitute = stated with { FamilyName = SymbolFontRecode.SubstituteFamily };
        if (Face(substitute) is not { } symbolFace) return null;

        FontReference? symbolReference = _references.GetValueOrDefault(substitute.FaceKey);
        if (symbolReference is null
            || !SymbolFontRecode.IsSubstituteFamily(symbolReference.FamilyName))
        {
            return null;
        }

        return (recoded.ToString(), symbolFace, symbolReference);
    }

    /// <summary>
    /// The size the label is set at: the level's own when it states one, and the item's text otherwise.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A level's <c>w:lvl/w:rPr</c> is character formatting for the label alone, and <c>w:sz</c> in it is
    /// regularly a different size from the item's text — which is why the level's <c>w:rFonts</c> is
    /// already read here (see <see cref="LabelFace"/>) and why the size belongs beside it. LibreOffice
    /// reads the whole of that <c>w:rPr</c> into the level's character style
    /// (<c>writerfilter/source/dmapper/NumberingManager.cxx</c>, <c>ListLevel::GetCharStyle</c>) and its
    /// export writes it back out as a <c>WW8NumNz</c> style, which is where a flat-ODF round trip shows
    /// it.
    /// </para>
    /// <para>
    /// A bigger label makes the item's first line taller — see
    /// <see cref="PageParagraph.LabelRaisesFirstLine"/> — so this is not only a matter of how wide the
    /// label is drawn.
    /// </para>
    /// </remarks>
    private static Length LabelSize(WordNumberingLevel definition, WordTextStyle text)
        => Word.Attribute(Word.Child(definition.RunProperties, "sz"), "val") is { } stated
           && Word.Integer(stated, out int halfPoints)
           && halfPoints > 0
            ? Length.FromPoints(halfPoints / 2.0)
            : text.Size;

    /// <summary>The position of a level's <c>w:tab w:val="num"</c>, or null when it states none.</summary>
    private static Length? NumberingTab(XElement? levelProperties)
    {
        foreach (XElement tab in Word.Children(Word.Child(levelProperties, "tabs"), "tab"))
        {
            if (Word.Attribute(tab, "val") != "num") continue;
            if (Twips(tab, "pos") is { } position) return position;
        }
        return null;
    }

    /// <summary>One twips-valued attribute, or null when it is absent or unreadable.</summary>
    private static Length? Twips(XElement? element, string name)
        => Word.Attribute(element, name) is { } value
           && Word.Long(value, out long twips)
            ? Length.FromTwips(twips)
            : null;
}
