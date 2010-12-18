Revoked certificates:

- User 2 Sub CA 1 Root 8


Start the OCSP responder:
openssl ocsp -index demoCA/index.txt -port 8889 -rsigner demoCA/cacert.pem -rkey demoCA/private/cakey.pem -CA demoCA/cacert.pem -text





	
