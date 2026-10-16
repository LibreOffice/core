using XRay.Office.Core.Formats;

namespace XRay.Office.Core;

/// <summary>Base class for every exception X-Ray.Office raises deliberately.</summary>
public class OfficeException : Exception
{
    /// <summary>Creates the exception.</summary>
    public OfficeException(string message) : base(message) { }

    /// <summary>Creates the exception with an inner cause.</summary>
    public OfficeException(string message, Exception innerException) : base(message, innerException) { }
}

/// <summary>
/// The document's format is not one X-Ray.Office can read.
/// </summary>
public sealed class UnsupportedFormatException : OfficeException
{
    /// <summary>Creates the exception.</summary>
    /// <param name="format">The format that was identified, if any.</param>
    /// <param name="message">An explanation.</param>
    public UnsupportedFormatException(DocumentFormat format, string message) : base(message)
        => Format = format;

    /// <summary>The format that was identified, or <see cref="DocumentFormat.Unknown"/>.</summary>
    public DocumentFormat Format { get; }
}

/// <summary>
/// The document is damaged badly enough that lenient parsing could not recover it.
/// </summary>
/// <remarks>
/// X-Ray.Office repairs what it can and records the rest as
/// <see cref="Diagnostics.Diagnostic"/>s, so this is reserved for genuinely
/// unreadable input — a truncated container, an unparseable directory — rather than
/// for ordinary specification violations.
/// </remarks>
public sealed class MalformedDocumentException : OfficeException
{
    /// <summary>Creates the exception.</summary>
    public MalformedDocumentException(string message) : base(message) { }

    /// <summary>Creates the exception with an inner cause.</summary>
    public MalformedDocumentException(string message, Exception innerException)
        : base(message, innerException) { }
}

/// <summary>
/// The document is encrypted and cannot be read without a password.
/// </summary>
public sealed class PasswordRequiredException : OfficeException
{
    /// <summary>Creates the exception.</summary>
    /// <param name="message">An explanation.</param>
    /// <param name="passwordWasSupplied">
    /// True when a password was given but rejected, false when none was given at all.
    /// </param>
    public PasswordRequiredException(string message, bool passwordWasSupplied)
        : base(message) => PasswordWasSupplied = passwordWasSupplied;

    /// <summary>
    /// True when a password was supplied and turned out to be wrong; false when no
    /// password was supplied.
    /// </summary>
    public bool PasswordWasSupplied { get; }
}
