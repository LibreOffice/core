# Collabora Office help build

Collabora Office can build two kinds of help output from helpcontent2:

* HTML (local)
* HTML (online)

The local help can be included in help-packs, which might be bundled with the
installer or provided as separate packages. They are supposed to be installed
by a user to provide localized help. Both help types are displayed in a browser.

## configure options

--with-help: selects the help type to build
--with-omindex: prepares the online help for a Xapian Omega based search index
--with-helppack-integration: includes the help-packs in the installer

Also see `configure --help`.

## HTML help, local

This type is displayed in a browser.

## HTML help, online

This help is the external one, currently provided by help.collaboraoffice.com. It
differs from the bundled HTML help in three main aspects:

* has a language chooser widget
* provides an additional Xapian Omega, CGI- / server-based search index
* includes some additional multimedia assets

It is opened in a browser.
