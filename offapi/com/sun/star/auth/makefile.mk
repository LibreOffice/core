PRJ = ..$/..$/..$/..
PRJNAME = auth

TARGET = cssauth
PACKAGE = com$/sun$/star$/auth

.INCLUDE: $(PRJ)$/util$/makefile.pmk

IDLFILES = \
    AuthenticationFailedException.idl	\
    InvalidArgumentException.idl		\
    InvalidContextException.idl			\
    InvalidCredentialException.idl		\
    InvalidPrincipalException.idl		\
    PersistenceFailureException.idl		\
    UnsupportedException.idl			\
    SSOManagerFactory.idl				\
    SSOPasswordCache.idl				\
    XSSOAcceptorContext.idl				\
    XSSOContext.idl						\
    XSSOInitiatorContext.idl			\
    XSSOManager.idl						\
    XSSOManagerFactory.idl				\
    XSSOPasswordCache.idl


.INCLUDE: target.mk
.INCLUDE: $(PRJ)$/util$/target.pmk
