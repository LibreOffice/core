using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Spreadsheets.Layout;

/// <summary>
/// What a conditional format changes about the text of one cell, over whatever the cell states.
/// </summary>
/// <remarks>
/// <para>
/// A differential record rather than a whole <see cref="SheetCellFormat"/>, because that is the
/// shape the file states and the shape the reference applies. A SpreadsheetML <c>&lt;dxf&gt;</c>
/// holds only the properties the rule overrides, and Calc turns each one into a cell
/// <em>style</em> whose item set holds exactly those items; everything the style is silent about
/// falls through to the cell's own pattern. So a rule that states a colour and nothing else must
/// leave the size, the face and the weight alone, and a null here is that silence.
/// </para>
/// <para>
/// <strong>One rule wins outright — the attributes of two matching rules are not merged.</strong>
/// <c>ScDocument::GetCondResult</c> (<c>sc/source/core/data/documen4.cxx</c>) walks the formats a
/// cell is covered by, asks each for a style name, and returns the <em>first</em> non-empty one's
/// item set; <c>ScConditionalFormat::GetCellStyle</c> (<c>conditio.cxx</c>) likewise returns the
/// first matching entry's style within a format. A reader that unioned two rules' properties
/// would paint combinations the reference never draws.
/// </para>
/// <para>
/// Deliberately holds nothing about fills or borders. A conditional fill is a
/// <em>decoration</em> and goes through <see cref="SheetFormatting.SetConditionalBackground"/>,
/// which is the layer that already exists for a colour scale and the layer that
/// <see cref="SheetDecorationArea"/> is kept away from — a conditional format must not extend how
/// far a sheet prints.
/// </para>
/// </remarks>
public readonly record struct SheetConditionalText
{
    /// <summary>The colour the text is drawn in, or null to keep the cell's own.</summary>
    public Colour? Colour { get; init; }

    /// <summary>Whether a line is drawn through the text, or null to keep the cell's own.</summary>
    /// <remarks>
    /// Both directions are needed and the corpus proves it: 287 of the 413 rules on
    /// <c>TK-Syllabus-Comparison-Document-v2.xlsx</c> state <c>&lt;strike val="0"/&gt;</c>, which
    /// <em>removes</em> the strikethrough the cell states of its own, and 26.2.4.2's own view of
    /// that file writes those rules out as <c>style:text-line-through-style="none"</c>.
    /// </remarks>
    public bool? IsStruckThrough { get; init; }

    /// <summary>The weight on the usual 100–900 scale, or null to keep the cell's own.</summary>
    public int? FontWeight { get; init; }

    /// <summary>Whether the face is italic, or null to keep the cell's own.</summary>
    public bool? IsItalic { get; init; }

    /// <summary>The line under the text, or null to keep the cell's own.</summary>
    public SheetUnderline? Underline { get; init; }

    /// <summary>The em size, or null to keep the cell's own.</summary>
    public Length? FontSize { get; init; }

    /// <summary>The face, or null to keep the cell's own.</summary>
    public string? FontFamily { get; init; }

    /// <summary>
    /// The generic class the rule's face was declared with, for the substitution its name alone
    /// does not settle, or null to keep the cell's own.
    /// </summary>
    /// <remarks>
    /// It travels with <see cref="FontFamily"/> and only with it: a rule that replaces the face
    /// replaces what the face falls back to, and one that is silent about the face must leave
    /// both alone. An ODF conditional style carries the pair on its own
    /// <c>style:text-properties</c> exactly as a cell style does, so the reader has it to hand;
    /// a SpreadsheetML <c>&lt;dxf&gt;</c> states no such thing and leaves this null.
    /// </remarks>
    public FontFamilyClass? DeclaredFontClass { get; init; }

    /// <summary>True when the rule changes nothing about the text.</summary>
    public bool IsNone
        => Colour is null && IsStruckThrough is null && FontWeight is null && IsItalic is null
           && Underline is null && FontSize is null && FontFamily is null
           && DeclaredFontClass is null;

    /// <summary>Applies this rule's properties over what a cell states.</summary>
    /// <param name="stated">The format the cell would be drawn in with no rule matching.</param>
    public SheetCellFormat Over(SheetCellFormat stated)
    {
        ArgumentNullException.ThrowIfNull(stated);
        if (IsNone) return stated;

        return stated with
        {
            Colour = Colour ?? stated.Colour,
            IsStruckThrough = IsStruckThrough ?? stated.IsStruckThrough,
            FontWeight = FontWeight ?? stated.FontWeight,
            IsItalic = IsItalic ?? stated.IsItalic,
            Underline = Underline ?? stated.Underline,
            FontSize = FontSize ?? stated.FontSize,
            FontFamily = FontFamily ?? stated.FontFamily,
            DeclaredFontClass = DeclaredFontClass ?? stated.DeclaredFontClass,
        };
    }
}
