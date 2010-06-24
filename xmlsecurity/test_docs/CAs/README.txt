This folder contains the directory structures needed by openssl to create
and manager certificates.



################################################################################

Root 1
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 1 
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

User 1 Sub CA 1 Root 1 
======================
Valid end certificate.
CRLDP = NO
AIA = NO

################################################################################

Root 2
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 2
===============
Valid intermediate CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

User 1 Sub CA 1 Root 2
======================
Valid end certificate.
CRLDP = NO
AIA = NO

################################################################################

Root 3
======
Valid root CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 3
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

User 1 Sub CA 1 Root 3
======================
Valid end certificate.
CRLDP = NO
AIA = NO

################################################################################

Root 4
======
Valid root CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO

Revoked certificates:

- Sub CA 2 Root 4


Sub CA 1 Root 4
===============
Valid intermediate CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

Revoked certificates:

- User 2 Sub CA 1 Root 4


User 1 Sub CA 1 Root 4
======================
Valid end certificate.
CRLDP = NO
AIA = NO

User 2 Sub CA 1 Root 4
======================
Revoked end certificate.
CRLDP = NO
AIA = NO

Sub CA 2 Root 4
===============
Revoked intermediate CA certificate. Reason = keyCompromise
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

Revoked certificates:

- User 2 Sub CA 2 Root 4

User 1 Sub CA 2 Root 4
======================
Valid end certificate. Issuer is revoked.
CRLDP = NO
AIA = NO

User 2 Sub CA 2 Root 4
======================
Revoked end certificate. Reason = keyCompromise. Issuer is revoked.
CRLDP = NO
AIA = NO

################################################################################

Root 5
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 5
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO
CRLDP = URL=http://localhost:8901/demoCA/crl/Root_5.crl
AIA = NO

User 1 Sub CA 1 Root 5
======================
Valid end certificate.
CRLDP = URL=http://localhost:8902/demoCA/crl/Sub_CA_1_Root_5.crl
AIA = NO


################################################################################

Root 6
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO

Sub CA 1 Root 6
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = URL=http://localhost:8901/demoCA/crl/Root_6.crl
AIA = NO

User 1 Sub CA 1 Root 6
======================
Valid end certificate.
CRLDP = URL=http://localhost:8902/demoCA/crl/Sub_CA_1_Root_6.crl
AIA = NO

################################################################################

Root 7
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO

Revoked certificates:

- Sub CA 2 Root 7


Sub CA 1 Root 7
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO
CRLDP = URL=http://localhost:8901/demoCA/crl/Root_7.crl
AIA = NO

Revoked certificates:

- User 2 Sub CA 1 Root 7


User 1 Sub CA 1 Root 7
======================
Valid end certificate.
CRLDP = URL=http://localhost:8902/demoCA/crl/Sub_CA_1_Root_7.crl
AIA = NO

User 2 Sub CA 1 Root 7
======================
Revoked end certificate. Reason = CACompromise.
CRLDP =  URL=http://localhost:8902/demoCA/crl/Sub_CA_1_Root_7.crl
AIA = NO

Sub CA 2 Root 7
===============
Revoked intermediate CA certificate. Reason = keyCompromise
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO
CRLDP = URL=http://localhost:8901/demoCA/crl/Root_7.crl
AIA = NO

Revoked certificates:

- User 2 Sub CA 2 Root 7

User 1 Sub CA 2 Root 7
======================
Valid end certificate. Issuer is revoked.
CRLDP = URL=http://localhost:8902/demoCA/crl/Sub_CA_2_Root_7.crl
AIA = NO

User 2 Sub CA 2 Root 7
======================
Revoked end certificate. Reason = CACompromise. Issuer is revoked.
CRLDP = URL=http://localhost:8902/demoCA/crl/Sub_CA_2_Root_7.crl
AIA = NO

################################################################################

Root 8
======
Valid root CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = YES

Revoked certificates:

- Sub CA 2 Root 8


Sub CA 1 Root 8
===============
Valid intermediate CA certificate.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = YES
CRLDP = 
AIA = OCSP;URI:http://localhost:8888

Revoked certificates:

- User 2 Sub CA 1 Root 8


User 1 Sub CA 1 Root 8
======================
Valid end certificate.
CRLDP = NO
AIA = OCSP;URI:http://localhost:8889

User 2 Sub CA 1 Root 8
======================
Revoked end certificate. Reason = superseded.
CRLDP = NO
AIA = OCSP;URI:http://localhost:8889

Sub CA 2 Root 8
===============
Revoked intermediate CA certificate. Reason = superseded.
Provides CRL = NO
Provides CRL via HTTP = NO
Provides OCSP = YES
CRLDP = NO
AIA = OCSP;URI:http://localhost:8888

Revoked certificates:

- User 2 Sub CA 2 Root 8

User 1 Sub CA 2 Root 8
======================
Valid end certificate. Issuer is revoked.
CRLDP = NO
AIA = OCSP;URI:http://localhost:8889

User 2 Sub CA 2 Root 8
======================
Revoked end certificate. Reason = superseded. Issuer is revoked.
CRLDP = NO
AIA =  OCSP;URI:http://localhost:8889

################################################################################

Root 9
======
Valid root CA certificate. (Not installed.)
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 9
===============
Valid intermediate CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO
CRLDP = NO
AIA = NO

User 1 Sub CA 1 Root 9
======================
Valid end certificate.
CRLDP = NO
AIA = NO

################################################################################

Root 10
======
Valid root CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO

Sub CA 1 Root 10
===============
Valid intermediate CA certificate. (Not installed.)
Provides CRL = NO
Provides CRL via HTTP = YES
Provides OCSP = NO
CRLDP = NO
AIA = NO

User 1 Sub CA 1 Root 10
======================
Valid end certificate.
CRLDP = URI:http://localhost:8902/demoCA/crl/Sub_CA_1_Root_10.crl
AIA = caIssuers;URI:http://localhost:8910/demoCA/Sub_CA_1_Root_10.crt

################################################################################

Root 11
======
Valid root CA certificate.
Provides CRL = YES
Provides CRL via HTTP = NO
Provides OCSP = NO

User x Root 11
======================
All certificate issues by Root 11 are valid end certificate. Currently there are
31 certificates.

