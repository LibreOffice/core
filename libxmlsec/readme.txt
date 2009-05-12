The XML Security library has been modified, so that there is NO verification
of the certificate during sign or verification operation. On Windows this was 
done in the function xmlSecMSCryptoX509StoreVerify (file 
src/mscrypto/x509vfy.c) and on UNIX in xmlSecNssX509StoreVerify 
(file src/nss/x509vfy.c).

This change requires that the XML Signature contains in 
Signature/KeyInfo/X509Data only entries which represent the same 
certificate.
The implementation creates certificates from all of the X509Data children
(X509IssuerSerial, X509Certificate) and used to iterate over all certificates,
verify them and return the first "good" certificate. Now the first one is 
used.

The X509IssuerSerial information is used by XML Security Library to find the 
certificate in the certificate store on the machine. The X509Certificate entry
is used to create a certificate no matter if this is already contained in the
certificate store.

Do not forget: Suggest to XML Security Library to provide a way to carry out 
signature operations without verification of certificates. There is flag
XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS that can be set in a 
xmlSecKeyInfoCtx (see function xmlSecNssKeyDataX509XmlRead, in file src/nss/x509.c),
which indicates such a possibility but it does not work.
