Revoked certificates:

- Sub CA 2 Root 8


Start the OCSP responder:
openssl ocsp -index demoCA/index.txt -port 8888 -rsigner demoCA/cacert.pem -rkey demoCA/private/cakey.pem -CA demoCA/cacert.pem -text


