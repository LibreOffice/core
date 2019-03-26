Wrappers so you can use all the Java Runtime Environments with their slightly incompatible APIs with more ease.

Used to use an over-engineered "plugin" mechanism although there was only one
"plugin", called "sunmajor", that handles all possible JREs.

IMPORTANT:  The <updated> element in vmfwk/distributions/OpenOfficeorg/javavendors_*.xml files
should only be updated for incompatible changes, not for compatible ones.  As stated in the commit
message of <https://gerrit.libreoffice.org/#/c/69730/> "javavendors_*.xml <updated> should not have
been updated...":  "Changing <updated> causes jfw_startVM and jfw_getSelectedJRE (both
jvmfwk/source/framework.cxx) to fail with JFW_E_INVALID_SETTINGS, which in turn causes functionality
that requires a JVM to issue a GUI error dialog stating that the user must select a new JRE in the
Options dialog.  While that behavior makes sense if a JRE was selected that would no longer be
supported by the modified javavendors_*.xml, it is just annoying if an already selected JRE is still
supported.  And a compatible change to javavendors_*.xml implies that an already selected JRE will
still be supported."
