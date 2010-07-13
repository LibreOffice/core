The XML Security library has been modified, so that there is NO verification of
the certificate during sign or verification operation. On Windows this was done
in the function xmlSecMSCryptoX509StoreVerify (file src/mscrypto/x509vfy.c) and
on UNIX in xmlSecNssX509StoreVerify (file src/nss/x509vfy.c).

The implementation creates certificates from all of the X509Data children, such
as X509IssuerSerial and X509Certificate and stores them in a certificate store
(see xmlsec/src/mscrypto/x509.c:xmlSecMSCryptoX509DataNodeRead). It must then
find the certificate containing the public key which is used for validation
within that store. This is done in xmlSecMSCryptoX509StoreVerify. This function
however only takes those certificates into account which can be validated. This
was changed by the patch xmlsec1-noverify.patch, which prevents this certificate
validation.

xmlSecMSCryptoX509StoreVerify iterates over all certificates contained or
referenced in the X509Data elements and selects one which is no issuer of any of
the other certificates. This certificate is not necessarily the one which was
used for signing but it must contain the proper validation key, which is
sufficient to validate the signature. See 
http://www.w3.org/TR/xmldsig-core/#sec-X509Data 
for details.

There is a flag XMLSEC_KEYINFO_FLAGS_X509DATA_DONT_VERIFY_CERTS that can be set
in a xmlSecKeyInfoCtx (see function xmlSecNssKeyDataX509XmlRead, in file
src/nss/x509.c), which indicates that one can turn off the validation. However,
setting it will cause that the validation key is not found. If the flag is set,
then the key is not extracted from the certificate store which contains all the
certificates of the X509Data elements. In other words, the certificates which
are delivered within the XML signature are not used when looking for suitable
validation key.


