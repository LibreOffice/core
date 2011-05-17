Security related test documents readme

--------- autotest.p12 ---------

This is a dummy certificate for OpenOffice.org / StarOffice automated testing.
The password is "autotest"

Usage:
Import the certificate in either
- Mozilla
- Thunderbird or
- Firebird

When importing the certificate you will be asked to set a password.
Use "autotest" here as well, if you forget your password, you won't
be able to get rid of the certificate anymore.

Go back to the office.

The application will then find the certificate the next time you go to 
Tools -> Macros ... -> Digital Signatures and click the button 
"import". You will be prompted for the password, enter "autotest".
Now you can work with certificates.

Note: The certificate is time-limited for two years. When it expires,
a new certificate will have to be generated and the documents might
have to be refreshed as well (don't know for sure)


------- test_macro.odt ------------

This is a document that contains a macro in the file content.xml which was a 
security breach in some older Office version
