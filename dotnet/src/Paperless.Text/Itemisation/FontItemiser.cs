using Paperless.Text.Fonts;

namespace Paperless.Text.Itemisation;

/// <summary>A stretch of text that one face can draw.</summary>
/// <param name="Start">Its first character, as an index into the text.</param>
/// <param name="Length">How many UTF-16 code units it covers.</param>
/// <param name="Face">The face that will draw it.</param>
/// <param name="IsFallback">True when this is not the face the run asked for.</param>
public readonly record struct FaceRun(int Start, int Length, OpenTypeFace Face, bool IsFallback)
{
    /// <summary>One past the run's last character.</summary>
    public int End => Start + Length;
}

/// <summary>
/// Splits a run further where its own face has no glyph for what it contains.
/// </summary>
/// <remarks>
/// <para>
/// The half of font fallback that was missing: coverage has been queryable since the OpenType reader
/// was written, and choosing the replacement and cutting the run at it is what turns a query into a
/// rendered page. Without it a run set in a face that lacks, say, Hebrew draws a row of
/// missing-glyph boxes at whatever width that face gives <c>.notdef</c> — which is both visibly
/// wrong and, because the width is wrong too, breaks the line in the wrong place.
/// </para>
/// <para>
/// LibreOffice does this after shaping rather than before: it lays the run out in the primary face,
/// collects the characters that came back as <c>.notdef</c>, and lays those out again in a fallback
/// face, stacking up to <c>MAX_FALLBACK</c> layouts in a <c>MultiSalLayout</c>
/// (<c>OutputDevice::ImplGlyphFallbackLayout</c>, <c>vcl/source/outdev/font.cxx</c>). Splitting
/// beforehand on the <c>cmap</c> gives the same partition for everything but a font whose
/// <c>cmap</c> claims a character its outlines do not have, and it costs one pass instead of two.
/// </para>
/// <para>
/// A non-spacing mark is kept with what it is attached to wherever it can be. A base and its mark
/// shaped in two different faces is not a mark on that base — it is a mark on nothing, positioned by
/// a font that never saw the letter — so the mark follows the preceding face whenever that face can
/// draw it at all.
/// </para>
/// </remarks>
public static class FontItemiser
{
    /// <summary>
    /// Splits a range of text into the faces that can draw it.
    /// </summary>
    /// <param name="text">The text the range indexes into.</param>
    /// <param name="start">The range's first character.</param>
    /// <param name="length">How many characters it covers.</param>
    /// <param name="primary">The face the run asked for.</param>
    /// <param name="fallback">Where to look when the primary face has no glyph, or null to not look.</param>
    /// <param name="report">Called once per character that needed a fallback, resolved or not.</param>
    public static List<FaceRun> Split(
        ReadOnlySpan<char> text,
        int start,
        int length,
        OpenTypeFace primary,
        IGlyphFallbackResolver? fallback,
        Action<GlyphFallback>? report = null)
    {
        ArgumentNullException.ThrowIfNull(primary);

        List<FaceRun> runs = [];
        if (length <= 0) return runs;

        if (fallback is null)
        {
            runs.Add(new FaceRun(start, length, primary, IsFallback: false));
            return runs;
        }

        int end = start + length;
        int runStart = start;
        OpenTypeFace runFace = primary;
        bool runIsFallback = false;
        bool started = false;

        for (int at = start; at < end;)
        {
            int width = 1;
            int codePoint = text[at];
            if (char.IsHighSurrogate(text[at]) && at + 1 < end && char.IsLowSurrogate(text[at + 1]))
            {
                codePoint = char.ConvertToUtf32(text[at], text[at + 1]);
                width = 2;
            }

            OpenTypeFace face = primary;
            bool isFallback = false;

            if (!primary.HasGlyphFor(codePoint))
            {
                // A mark the current face can draw stays with the base it is attached to, whatever
                // the primary face says: the mark is positioned against the base's outline.
                bool markOnCurrent = started
                    && BidiProperties.ClassOf(codePoint) == BidiClass.NSM
                    && runFace.HasGlyphFor(codePoint);

                if (markOnCurrent)
                {
                    face = runFace;
                    isFallback = runIsFallback;
                }
                else if (fallback.FallbackFor(codePoint, primary.Weight, primary.IsItalic) is { } found)
                {
                    face = found;
                    isFallback = true;
                    report?.Invoke(new GlyphFallback(codePoint, primary.FamilyName, found.FamilyName));
                }
                else
                {
                    // Nothing installed has it. The primary face draws its missing-glyph box, which
                    // is what LibreOffice does too once its fallback chain is exhausted — but the
                    // caller is told, because a box on a page is worth explaining.
                    report?.Invoke(new GlyphFallback(codePoint, primary.FamilyName, null));
                }
            }

            if (!started)
            {
                runFace = face;
                runIsFallback = isFallback;
                started = true;
            }
            else if (!ReferenceEquals(face, runFace))
            {
                runs.Add(new FaceRun(runStart, at - runStart, runFace, runIsFallback));
                runStart = at;
                runFace = face;
                runIsFallback = isFallback;
            }

            at += width;
        }

        runs.Add(new FaceRun(runStart, end - runStart, runFace, runIsFallback));
        return runs;
    }
}
