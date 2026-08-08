using System.Xml.Linq;
using Paperless.Core.Graphics;
using Paperless.Core.Units;
using Paperless.WordProcessing.Layout;

namespace Paperless.WordProcessing.Ooxml;

/// <content>
/// Reading <c>w:pBdr</c> — the rules round a paragraph, which draw and also take room.
/// </content>
/// <remarks>
/// <para>
/// Two properties of it decide the whole design, and both were measured off LibreOffice's own PDFs rather
/// than taken from the specification.
/// </para>
/// <list type="bullet">
///   <item>
///     <b>It is inherited side by side.</b> A paragraph whose style states a bottom rule and which states
///     only a top of its own keeps both — <c>PBorderHandler</c> turns each side into its own property, so
///     the style chain merges them individually exactly as <c>w:spacing</c>'s attributes do. Taking the
///     innermost <c>w:pBdr</c> whole instead loses every side the paragraph did not restate, which on this
///     corpus is most of them: 294 of the 1078 <c>w:pBdr</c> elements in the words track are in
///     <c>styles.xml</c>.
///   </item>
///   <item>
///     <b>A stated <c>w:val="none"</c> is an answer, not a silence.</b> It beats the style chain below it
///     and takes no room, which is how a header paragraph switches off the rule its style would give it —
///     measured on a probe where the style's bottom rule vanishes and the paragraph below moves up by
///     exactly the allowance.
///   </item>
/// </list>
/// <para>
/// <c>w:space</c> is in <em>points</em> here, which is the one place in WordprocessingML that unit appears
/// on a border: <c>w:sz</c> beside it is in eighths of a point and every length in <c>w:tblBorders</c> is
/// twips. Reading it as twips puts a 10 pt distance at half a point.
/// </para>
/// </remarks>
public sealed partial class DocxLayoutSource
{
    /// <summary>
    /// The rules round a paragraph, resolved side by side down its style chain, or null when it has none.
    /// </summary>
    /// <param name="properties">The paragraph's own <c>w:pPr</c>, or null.</param>
    private ParagraphBorderSet? ParagraphBorders(XElement? properties)
    {
        string? styleId = Word.Attribute(Word.Child(properties, "pStyle"), "val")
                          ?? _styles.DefaultStyleId(WordStyleType.Paragraph);

        List<XElement> layers = _styles.ParagraphPropertyLayers(
            "pBdr", properties, styleId, _tableStyle);

        if (layers.Count == 0) return null;

        ParagraphBorderSet set = new()
        {
            Left = Side("start", "left"),
            Right = Side("end", "right"),
            Top = Side("top", null),
            Bottom = Side("bottom", null),
            Between = Side("between", null),
        };

        return set.IsEmpty ? null : set;

        ParagraphBorder? Side(string side, string? legacySide)
        {
            foreach (XElement layer in layers)
            {
                if (ParagraphBorderOf(layer, side, legacySide) is { } found) return found;
            }

            return null;
        }
    }

    /// <summary>
    /// One side of a <c>w:pBdr</c>, or null when that layer states nothing about it.
    /// </summary>
    /// <remarks>
    /// A zero width and a null are different answers: the first is a stated <c>none</c> that stops the
    /// search, and the second lets the layer below speak. <c>w:sz</c> is in eighths of a point and
    /// <c>w:space</c> in whole points; a border whose colour resolves to nothing is still a border, so
    /// black is the fallback.
    /// </remarks>
    private ParagraphBorder? ParagraphBorderOf(XElement? borders, string side, string? legacySide)
    {
        XElement? stated =
            Word.Child(borders, side)
            ?? (legacySide is null ? null : Word.Child(borders, legacySide));

        if (stated is null) return null;

        Length space =
            Word.Integer(Word.Attribute(stated, "space"), out int points) && points > 0
                ? Length.FromPoints(points)
                : Length.Zero;

        if (Word.Attribute(stated, "val") is null or "none" or "nil")
        {
            return new ParagraphBorder(Length.Zero, Length.Zero, Colour.Black);
        }

        Length width =
            Word.Integer(Word.Attribute(stated, "sz"), out int eighths) && eighths > 0
                ? Length.FromPoints(eighths / 8.0)
                : HairlineBorder;

        Colour colour =
            WordThemeColour.Read(stated, _theme, "color", "themeColor", "themeTint", "themeShade")
            ?? Colour.Black;

        return new ParagraphBorder(width, space, colour);
    }
}
