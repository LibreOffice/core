# .NET UNO Runtime Environment

.NET assemblies for the newer cross-platform .NET UNO binding.

Contains code for the `net_basetypes` assembly, `net_bridge` assembly, and `net_bootstrap` native library, along with unit tests.
Includes scripts to build the `net_uretypes` and `net_oootypes` assemblies from IDL, using `netmaker` from the codemaker/ module.
Also includes scripts to build the `LibreOffice.Bindings` nuget package for the SDK.

The assemblies compile and can be consumed against .NET Standard 2.0, although the build requirement is set to .NET SDK 8.0 or higher for tests and examples, and to ensure cross platform uniformity.