Tests for using the default URL for update information. This URL is currently contained in
the version.ini (ExtensionUpdateURL) and is used to obtain update information for extensions which do not provide
an URL themselves.

The extensions default1.oxt and default2.oxt do not have a URL for update information.

To test this one has to put this entry into the version.ini:

ExtensionUpdateURL=http://extensions.openoffice.org/testarea/desktop/default_url/update/feed1.xml
