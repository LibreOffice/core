namespace Paperless.Containers.Encryption;

/// <summary>
/// Decrypts a password-protected document into a readable stream.
/// </summary>
/// <remarks>
/// The supported formats between them use at least five unrelated schemes — ODF
/// Blowfish-CFB and AES, OOXML agile and standard, and legacy RC4 and XOR
/// obfuscation. See <c>dotnet/research/05-infrastructure.md</c> section C.
/// <para>
/// Note that "protected" does not always mean "encrypted": several formats also carry
/// a write-protection password that does not encrypt anything and must not be
/// mistaken for one. Those documents read without a password.
/// </para>
/// </remarks>
public interface IDecryptionProvider
{
    /// <summary>True when this provider recognises the container's encryption scheme.</summary>
    bool CanDecrypt(IPackage package);

    /// <summary>
    /// Decrypts the payload.
    /// </summary>
    /// <param name="package">The encrypted container.</param>
    /// <param name="password">The password to try.</param>
    /// <returns>A seekable stream over the decrypted document.</returns>
    /// <exception cref="Core.PasswordRequiredException">The password was wrong.</exception>
    Stream Decrypt(IPackage package, string? password);
}
