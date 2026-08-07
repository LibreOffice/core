using Paperless.Core.Extraction;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Core.Numbering;
using Paperless.Core.Units;
using Paperless.Presentations.Layout;
using Paperless.Text.Fonts;
using Paperless.Text.Layout;

namespace Paperless.Presentations.MsBinary;

/// <summary>
/// Turns a binary PowerPoint text run into the text body the slide layouter lays out.
/// </summary>
/// <remarks>
/// <para>
/// The counterpart of <c>PptxTextBody</c> and <c>OdfTextBody</c>, and the place the master style
/// sheet finally earns its keep: a PPT paragraph states only what differs from its outline level,
/// so its size, colour, typeface, indents, alignment and bullet are all resolved here, property by
/// property, against <see cref="PptStyleSheet"/>.
/// </para>
/// <para>
/// <strong>A property mask of zero is not a property of zero.</strong> Every field below is
/// carried on the run beside the mask bit that says whether the run stated it, because a
/// paragraph that says nothing about its alignment is left-aligned only if its master is —
/// reading the field regardless would left-align every inherited paragraph in the deck and,
/// worse, would move the text of every outline paragraph to the shape's edge by zeroing an
/// indent the master states.
/// </para>
/// </remarks>
internal static class PptTextBody
{
    /// <summary>The mask bit a paragraph sets when it states its own alignment.</summary>
    private const uint StatesAlignment = 0x0000_0800;

    /// <summary>The mask bit for the line feed.</summary>
    private const uint StatesLineFeed = 0x0000_1000;

    /// <summary>The mask bits for the space above and below.</summary>
    private const uint StatesSpaceBefore = 0x0000_2000;
    private const uint StatesSpaceAfter = 0x0000_4000;

    /// <summary>The mask bits for the two indents.</summary>
    private const uint StatesTextOffset = 0x0000_0100;
    private const uint StatesBulletOffset = 0x0000_0400;

    /// <summary>The mask bits for the bullet's own face, size and colour.</summary>
    private const uint StatesBulletFont = 0x0000_0010;
    private const uint StatesBulletHeight = 0x0000_0040;
    private const uint StatesBulletColour = 0x0000_0020;

    /// <summary>The mask bits a character run sets for its face, size and colour.</summary>
    private const uint StatesFontIndex = 0x0001_0000;
    private const uint StatesFontHeight = 0x0002_0000;
    private const uint StatesColour = 0x0004_0000;

    /// <summary>The mask bit for a raised or lowered baseline, <c>PPT_CharAttr_Escapement</c>.</summary>
    private const uint StatesEscapement = 0x0008_0000;

    /// <summary>
    /// Builds a body, or returns null when the run holds nothing to draw.
    /// </summary>
    /// <param name="run">The text run, as the reader produced it.</param>
    /// <param name="styles">The style sheet of the master the run's page belongs to.</param>
    /// <param name="scheme">The page's colour scheme, which every colour here resolves through.</param>
    /// <param name="fonts">The document's font table, which a typeface index refers to.</param>
    /// <param name="insets">The shape's text insets.</param>
    /// <param name="anchor">Where the block sits vertically.</param>
    /// <param name="wraps">Whether lines break at the shape's width.</param>
    /// <param name="autofits">
    /// Whether the text shrinks until it fits the shape. Decided by the shape rather than by the
    /// text — see <c>PptSlideLayout.Autofits</c> for what the binary format makes that mean.
    /// </param>
    public static SlideTextBody? Build(
        PptTextRun run,
        PptStyleSheet? styles,
        PptColourScheme scheme,
        PptFontTable fonts,
        Margins insets,
        TextAnchor anchor,
        bool wraps,
        bool autofits = false)
    {
        ArgumentNullException.ThrowIfNull(run);

        List<SlideParagraph> paragraphs = [];
        int start = 0;

        while (start <= run.Text.Length)
        {
            int stop = run.Text.IndexOf(PptTextReader.ParagraphSeparator, start);
            int length = (stop < 0 ? run.Text.Length : stop) - start;

            paragraphs.Add(Paragraph(run, styles, scheme, fonts, start, length));

            if (stop < 0) break;
            start = stop + 1;
        }

        // A run that ends with a return has one empty paragraph after it, which is an artefact of
        // the terminator rather than a paragraph the author wrote.
        if (paragraphs.Count > 1 && paragraphs[^1].Text.Length == 0)
        {
            paragraphs.RemoveAt(paragraphs.Count - 1);
        }

        if (paragraphs.Count == 0) return null;

        // EditEngine adds a paragraph's space above only when it is not the first, and its space
        // below only when it is not the last (ImpEditEngine::CalcHeight,
        // editeng/source/editeng/impedit2.cxx:4791-4802). Worth 0.125 pt on the corpus deck,
        // exactly one master unit — small, and the difference between "agrees with the reference"
        // and "nearly agrees".
        //
        // SlideTextLayout now applies the same rule for all three families, so this is redundant
        // rather than load-bearing. It stays because it makes the body this reader hands over say
        // what it means: the outer two spacings are not part of the text's height.
        paragraphs[0] = paragraphs[0] with { SpaceBefore = Length.Zero };
        paragraphs[^1] = paragraphs[^1] with { SpaceAfter = Length.Zero };

        return new SlideTextBody
        {
            Paragraphs = paragraphs,
            Insets = insets,
            Anchor = anchor,
            Wraps = wraps,
            AutoFit = autofits,
        };
    }

    private static SlideParagraph Paragraph(
        PptTextRun run,
        PptStyleSheet? styles,
        PptColourScheme scheme,
        PptFontTable fonts,
        int start,
        int length)
    {
        PptParagraphRun properties = PropertiesAt(run.Paragraphs, start);
        int depth = properties.Depth;

        PptParagraphLevel level = styles?.Paragraph(run.Kind, depth)
                                 ?? new PptParagraphLevel(0, 0x2022);
        PptCharacterLevel characters = styles?.Character(run.Kind, depth)
                                       ?? new PptCharacterLevel(0, 0, 0xFFFF, 18, 0x08000001, 0);

        // U+2028, which is what the PPTX and ODF readers already produce for a manual break, so
        // one layout rule serves all three. A newline breaks the same way — the break set accepts
        // both — but it is also what a reader may leave on the end of a paragraph's text to mean
        // the paragraph ends there, and the rule that gives a trailing break its own empty line
        // has to be able to tell those apart. Reading `\n` here left this deck's bullets a line
        // short each: `2015-Civil-Rights-Website-training.ppt` ends every paragraph in a `\x0B`.
        string text = run.Text.Substring(start, length).Replace(
            PptTextReader.LineBreak, '\u2028');

        List<SlideTextRun> runs = Runs(run, scheme, fonts, characters, start, length, text.Length);

        ushort alignment = properties.States(StatesAlignment) ? properties.Alignment : level.Alignment;
        short lineFeed = properties.States(StatesLineFeed) ? properties.LineFeed : level.LineFeed;
        short before = properties.States(StatesSpaceBefore) ? properties.SpaceBefore : level.SpaceBefore;
        short after = properties.States(StatesSpaceAfter) ? properties.SpaceAfter : level.SpaceAfter;
        ushort textOffset = properties.States(StatesTextOffset) ? properties.TextOffset : level.TextOffset;
        ushort bulletOffset = properties.States(StatesBulletOffset)
            ? properties.BulletOffset
            : level.BulletOffset;

        Length size = runs.Count > 0 ? runs[0].Size : Length.FromPoints(characters.FontHeight);

        return new SlideParagraph(
            text,
            runs,
            Alignment(alignment),
            Distance(before, size),
            Distance(after, size),
            Spacing(lineFeed),
            MasterUnits(textOffset),
            MasterUnits(bulletOffset) - MasterUnits(textOffset),
            Language: null,
            Marker: Marker(properties, level, scheme, fonts, runs))
        {
            // The master's own value, which PowerPoint writes as 0x240 — one inch — and which the
            // record's default already is. Reading it matters for the deck that states something
            // else, and stating nothing must not fall back to a word processor's half inch.
            DefaultTabInterval = level.DefaultTab > 0
                ? MasterUnits(level.DefaultTab)
                : SlideParagraph.DefaultTabDistance,
        };
    }

    /// <summary>
    /// The bullet the paragraph draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// The character, its face, its size and its colour each fall through to the master's level
    /// independently, which is what lets a deck state a per-level bullet once and every slide use
    /// it. The size is a percentage of the text's, so it becomes
    /// <see cref="SlideMarker.Scale"/> rather than a length.
    /// </remarks>
    private static SlideMarker? Marker(
        PptParagraphRun properties,
        PptParagraphLevel level,
        PptColourScheme scheme,
        PptFontTable fonts,
        List<SlideTextRun> runs)
    {
        bool bulleted = properties.HasBullet ?? level.HasBullet;
        if (!bulleted || runs.Count == 0) return null;

        char character = properties.BulletCharacter
                         ?? (level.BulletCharacter != 0 ? (char)level.BulletCharacter : '•');

        ushort font = properties.States(StatesBulletFont) ? properties.BulletFont : level.BulletFont;
        ushort height = properties.States(StatesBulletHeight)
            ? properties.BulletHeight
            : level.BulletHeight;
        uint colour = properties.States(StatesBulletColour)
            ? properties.BulletColour
            : level.BulletColour;

        char symbol = PptTextReader.Symbolised(character, fonts, font);
        string? face = fonts[font];

        // A face whose slots LibreOffice has a recode table for keeps both its code point and its
        // own name, because the two only mean anything together: `SlideTextLayout` turns the slot
        // into the OpenSymbol glyph holding the same picture, and it needs the face to know which
        // table to use. Anything else keeps the old answer — U+2022, drawn in the paragraph's own
        // face, since a symbol face's name with a non-symbol code point resolves to nothing.
        bool recodeable = fonts.IsSymbol(font) && SymbolFontRecode.IsRecodeable(face);

        string text = recodeable
            ? symbol.ToString()
            : OutlineNumbers.NormaliseBullet(symbol.ToString());
        if (text.Length == 0) return null;

        string? typeface = recodeable ? face : fonts.IsSymbol(font) ? null : face;

        return new SlideMarker(
            text,
            typeface,
            height is > 0 and <= 400 ? height / 100.0 : 1.0,
            PptColour.ResolveText(colour, scheme));
    }

    /// <summary>
    /// The runs covering a paragraph, each resolved against the master's level for what it does
    /// not state.
    /// </summary>
    private static List<SlideTextRun> Runs(
        PptTextRun run,
        PptColourScheme scheme,
        PptFontTable fonts,
        PptCharacterLevel level,
        int start,
        int length,
        int textLength)
    {
        List<SlideTextRun> runs = [];
        int end = start + length;
        int position = 0;

        // The properties in force at the paragraph's first character, kept for the empty-paragraph
        // case below: an empty paragraph covers no characters, so the loop places nothing from it,
        // and the run it sits inside is the only thing that says how tall its blank line is.
        PptCharacterRun atStart = default;
        bool found = false;

        foreach (PptCharacterRun character in run.Characters)
        {
            int runEnd = position + character.Length;
            int from = Math.Max(position, start);
            int to = Math.Min(runEnd, end);

            if (!found && start >= position && start < runEnd) { atStart = character; found = true; }
            if (to > from) runs.Add(Run(character, scheme, fonts, level, from - start, to - from));

            position = runEnd;
            if (position >= end) break;
        }

        // Text past the last stated run, and a run that states none at all, both take the level's
        // defaults. A writer that under-counts is commoner than one that over-counts, and dropping
        // the tail would lose the text rather than its formatting.
        int covered = 0;
        foreach (SlideTextRun placed in runs) covered += placed.Length;

        if (covered < textLength)
        {
            runs.Add(Run(default, scheme, fonts, level, covered, textLength - covered));
        }

        // An empty paragraph still gets one run, of no characters, carrying the level's size.
        //
        // It is a blank *line*, not nothing: layout drops a paragraph that resolves no face at
        // all, so without this an empty paragraph contributes no height and everything below it
        // moves up by a line. PowerPoint decks use them as spacing constantly — the fourth page of
        // WC_Update-Aug03.ppt separates all eleven of its bullets that way, and LibreOffice's own
        // flat-ODF export of it writes each as a list header holding one empty paragraph.
        //
        // The PPTX reader has always done this; the comment on SlideParagraph.Runs says why.
        if (runs.Count == 0)
        {
            runs.Add(Run(atStart, scheme, fonts, level, 0, 0));
        }

        return runs;
    }

    private static SlideTextRun Run(
        PptCharacterRun character,
        PptColourScheme scheme,
        PptFontTable fonts,
        PptCharacterLevel level,
        int start,
        int length)
    {
        ushort fontIndex = character.States(StatesFontIndex) ? character.FontIndex : level.FontIndex;
        ushort height = character.States(StatesFontHeight) ? character.FontHeight : level.FontHeight;
        uint colour = character.States(StatesColour) ? character.Colour : level.Colour;

        RunEmphasis emphasis = (level.Emphasis & ~character.Stated)
                               | (character.Emphasis & character.Stated);

        // Already a percentage in the file, so it goes straight through; the size that goes with
        // it does not, and LibreOffice supplies DFLT_ESC_PROP whenever the value is non-zero
        // (filter/source/msfilter/svdfppt.cxx:5764-5775).
        short escapement =
            character.States(StatesEscapement) ? character.Escapement : level.Escapement;

        return new SlideTextRun(
            start,
            length,
            fonts[fontIndex],
            Length.FromPoints(height > 0 ? height : level.FontHeight),
            emphasis.HasFlag(RunEmphasis.Bold) ? 700 : 400,
            emphasis.HasFlag(RunEmphasis.Italic),
            PptColour.ResolveText(colour, scheme) ?? Colour.Black,
            IsUnderlined: emphasis.HasFlag(RunEmphasis.Underline),
            IsStruckThrough: emphasis.HasFlag(RunEmphasis.Strikethrough),
            Escapement: escapement == 0
                ? SlideEscapement.None
                : new SlideEscapement(escapement, SlideEscapement.AutomaticProportion));
    }

    /// <summary>
    /// The paragraph properties covering the character at <paramref name="start"/>.
    /// </summary>
    /// <remarks>
    /// A paragraph property run is <em>not</em> one paragraph: its count is a character count and
    /// a writer may cover several paragraphs with one run
    /// (<c>filter/source/msfilter/svdfppt.cxx:5081-5090</c>).
    /// </remarks>
    private static PptParagraphRun PropertiesAt(IReadOnlyList<PptParagraphRun> runs, int start)
    {
        int position = 0;

        foreach (PptParagraphRun run in runs)
        {
            position += Math.Max(run.Length, 1);
            if (start < position) return run;
        }

        return runs.Count > 0 ? runs[^1] : default;
    }

    private static TextAlignment Alignment(ushort adjust) => adjust switch
    {
        1 => TextAlignment.Centre,
        2 => TextAlignment.End,
        3 => TextAlignment.Justify,
        _ => TextAlignment.Start,
    };

    /// <summary>
    /// A line feed as a spacing rule.
    /// </summary>
    /// <remarks>
    /// One field, two units: a positive value is a percentage of the natural line height and a
    /// negative one is a fixed height in eighths of a point
    /// (<c>PPTParagraphObj::ApplyTo</c>, <c>svdfppt.cxx:6273-6289</c>). Reading the sign the wrong
    /// way turns a 92% paragraph into one set at eleven and a half points.
    /// </remarks>
    private static LineSpacingRule Spacing(short lineFeed)
        => lineFeed switch
        {
            0 => LineSpacingRule.SingleSpaced,
            > 0 => LineSpacingRule.Multiple(lineFeed / 100.0),
            _ => LineSpacingRule.Exactly(Length.FromPoints(-lineFeed / 8.0)),
        };

    /// <summary>
    /// The space above or below a paragraph, in the same two units as the line feed.
    /// </summary>
    /// <remarks>
    /// Negative is a distance in master units; positive is a proportion of the font's height, and
    /// LibreOffice's conversion of it is <c>fontHeight × value / 10</c> master units, with the
    /// height in points (<c>svdfppt.cxx:6300-6305</c>). Eighty master units make a point, so the
    /// whole of it reduces to a division by eighty — and the body style's default of 20 is a
    /// quarter of the font's size rather than a fifth of it.
    /// </remarks>
    private static Length Distance(short value, Length fontSize)
        => value <= 0
            ? MasterUnits((ushort)Math.Min(-value, ushort.MaxValue))
            : Length.FromEmu(fontSize.Emu * value / 80);

    private static Length MasterUnits(ushort units)
        => Length.FromEmu((long)units * Length.EmuPerInch / PptSlideLayout.MasterUnitsPerInch);
}
