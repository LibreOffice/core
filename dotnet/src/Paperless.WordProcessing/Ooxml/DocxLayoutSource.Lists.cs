using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;
using Paperless.Text.Shaping;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

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
        string? drawn = _numbering.Advance(numId, level);

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
            LabelFace(definition, text, face, drawn);

        PageLabel label = PageLabel.Measured(
            labelText, labelFace, LabelSize(definition, text),
            new ShapingOptions(Language: text.Language, DisableKerning: !text.AutoKerning));

        return (
            label with
            {
                Font = labelFont,
                Colour = text.Colour ?? Core.Graphics.Colour.Black,
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
    /// </remarks>
    private (string NumId, int Level)? ResolveNumbering(XElement? properties)
    {
        XElement? numbering = Word.Child(properties, "numPr");
        string? numId = Word.Value(numbering, "numId");
        int level = int.TryParse(Word.Value(numbering, "ilvl"), out int parsed) ? parsed : 0;

        string? styleId = Word.Attribute(Word.Child(properties, "pStyle"), "val");

        if (numId is null && styleId is not null)
        {
            WordProperty fromStyle = _styles.ResolveInStyleChain(
                styleId, WordStyleType.Paragraph, runProperty: false, "numPr");

            if (fromStyle.HasValue)
            {
                numId = Word.Value(fromStyle.Element, "numId");
                level = int.TryParse(Word.Value(fromStyle.Element, "ilvl"), out int fromStyleLevel)
                    ? fromStyleLevel
                    : 0;
            }
            else if (_numbering.FindInstanceForStyle(styleId) is { } styleLinked)
            {
                numId = styleLinked;
            }
        }

        if (numId is null or "0") return null;
        return (numId, Math.Clamp(level, 0, WordNumbering.LevelCount - 1));
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
    /// <see cref="Symbol"/>, which decides between the cases; U+2022 in the paragraph's own face is
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
        WordNumberingLevel definition, WordTextStyle text, OpenTypeFace face, string drawn)
    {
        FontReference? own = _references.GetValueOrDefault(text.FaceKey);

        string? family = WordParagraphFormats.SlotFamily(
            Word.Child(definition.RunProperties, "rFonts"), _theme?.Fonts, "ascii", "asciiTheme");

        if (definition.LevelText is [>= '\uE000' and <= '\uF8FF' and var slot])
        {
            return Symbol(family, slot) ?? (drawn, face, own);
        }

        if (family is not { Length: > 0 }) return (drawn, face, own);

        WordTextStyle named = text with { FamilyName = family };

        return Face(named) is { } resolved
            ? (drawn, resolved, _references.GetValueOrDefault(named.FaceKey))
            : (drawn, face, own);
    }

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
    private (string Text, OpenTypeFace Face, FontReference? Font)? Symbol(string? family, char slot)
    {
        if (family is not { Length: > 0 }) return null;
        if (!SymbolFontRecode.IsRecodeable(family)) return null;

        // Weight and italic are the level's own, and a symbol face has one of each; the size is
        // decided by LabelSize and plays no part in which file is loaded.
        WordTextStyle stated = new(family, Length.Zero, 400, false, null);
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
