namespace Paperless.Presentations.Ooxml;

/// <summary>
/// What a deck's automatic fields resolve to on one slide.
/// </summary>
/// <remarks>
/// <para>
/// The two an <c>a:fld</c> can ask for that are a property of the deck itself rather than of the
/// machine reading it. LibreOffice turns each into a real text field —
/// <c>com.sun.star.text.TextField.PageNumber</c> and <c>.PageCount</c>
/// (<c>oox/source/drawingml/textfield.cxx:107-117</c>) — so both are computed at draw time and
/// the value cached in the file is ignored.
/// </para>
/// <para>
/// <strong>The number is the slide's position, not anything the file states.</strong>
/// <c>p:presentation/@firstSlideNum</c> exists and LibreOffice does not read it; the token is in
/// <c>oox/source/token/tokens.txt</c> and nothing else in <c>oox</c> mentions it.
/// </para>
/// </remarks>
/// <param name="Number">The slide's one-based position in the deck.</param>
/// <param name="Count">How many slides the deck has.</param>
internal readonly record struct SlideFields(int Number, int Count);
