The file jvfwk3rc is intended for providing bootstrap parameter for the java 
framework within the build environment. It is not part of a product. Tools 
which are started in the environment, such as regcomp.exe and uno.exe, use
this rc file when Java is needed. 

The file javasettingsunopkginstall.xml only contains the root element of 
a settings file (<java ...>). It is a dummy which will be installed into
office/share/config/. Bundled extensions will used this file to store its
java settings. See framework.h bootstrap variable UNO_JAVA_JFW_INSTALL_DATA
