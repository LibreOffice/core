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

        (OpenTypeFace labelFace, FontReference? labelFont) = LabelFace(definition, text, face);

        PageLabel label = PageLabel.Measured(
            drawn, labelFace, LabelSize(definition, text),
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
    /// The face the label is set in: the level's own, unless its bullet had to be normalised.
    /// </summary>
    /// <remarks>
    /// A bullet level names a symbol font and states its bullet as a code point in that font's private
    /// use area, which means nothing anywhere else. <see cref="WordNumbering.FormatLabel"/> has already
    /// turned such a code point into U+2022, so keeping the level's font would draw a real bullet
    /// character through a font that has no such glyph — and the font is unlikely to be installed in any
    /// case. A level naming a font for an ordinary character keeps it.
    /// </remarks>
    /// <returns>
    /// The face and the reference it was resolved through. Both, because the reference cannot be
    /// recovered from the face afterwards \u2014 an <see cref="OpenTypeFace"/> is a parsed table
    /// directory and does not carry the path it was read from, and the path is the only thing a
    /// PDF can turn back into an embedded font program. A label drawn from a family-named
    /// reference is referenced and not embedded, which is what <c>pdffonts</c> reported for
    /// <c>word-features.docx</c>'s <c>LiberationSerif</c> and <c>OpenSymbol</c> labels while every
    /// body face in the same file embedded.
    /// </returns>
    private (OpenTypeFace Face, FontReference? Font) LabelFace(
        WordNumberingLevel definition, WordTextStyle text, OpenTypeFace face)
    {
        FontReference? own = _references.GetValueOrDefault(text.FaceKey);

        if (definition.LevelText is [>= '\uE000' and <= '\uF8FF']) return (face, own);

        string? family = WordParagraphFormats.SlotFamily(
            Word.Child(definition.RunProperties, "rFonts"), _theme?.Fonts, "ascii", "asciiTheme");
        if (family is not { Length: > 0 }) return (face, own);

        WordTextStyle named = text with { FamilyName = family };

        return Face(named) is { } resolved
            ? (resolved, _references.GetValueOrDefault(named.FaceKey))
            : (face, own);
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
           && int.TryParse(
               stated, System.Globalization.NumberStyles.Integer,
               System.Globalization.CultureInfo.InvariantCulture, out int halfPoints)
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
           && long.TryParse(value, System.Globalization.NumberStyles.Integer,
                            System.Globalization.CultureInfo.InvariantCulture, out long twips)
            ? Length.FromTwips(twips)
            : null;
}
