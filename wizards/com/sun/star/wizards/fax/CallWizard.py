import traceback

class CallWizard(object):

    '''
    Gives a factory for creating the service. This method is called by the
    <code>JavaLoader</code>
    <p></p>
    @param stringImplementationName The implementation name of the component.
    @param xMSF The service manager, who gives access to every known service.
    @param xregistrykey Makes structural information (except regarding tree
    structures) of a single registry key accessible.
    @return Returns a <code>XSingleServiceFactory</code> for creating the component.
    @see com.sun.star.comp.loader.JavaLoader#
    '''

    @classmethod
    def __getServiceFactory(self, stringImplementationName, xMSF, xregistrykey):
        xsingleservicefactory = None
        if stringImplementationName.equals(WizardImplementation.getName()):
            xsingleservicefactory = FactoryHelper.getServiceFactory(WizardImplementation, WizardImplementation.__serviceName, xMSF, xregistrykey)

        return xsingleservicefactory

    '''
    This class implements the component. At least the interfaces XServiceInfo,
    XTypeProvider, and XInitialization should be provided by the service.
    '''

    class WizardImplementation:
        __serviceName = "com.sun.star.wizards.fax.CallWizard"
        #private XMultiServiceFactory xmultiservicefactory

        '''
        The constructor of the inner class has a XMultiServiceFactory parameter.
        @param xmultiservicefactoryInitialization A special service factory could be
        introduced while initializing.
        '''

        @classmethod
        def WizardImplementation_XMultiServiceFactory(self, xmultiservicefactoryInitialization):
            tmp = WizardImplementation()
            tmp.WizardImplementation_body_XMultiServiceFactory(xmultiservicefactoryInitialization)
            return tmp

        def WizardImplementation_body_XMultiServiceFactory(self, xmultiservicefactoryInitialization):
            self.xmultiservicefactory = xmultiservicefactoryInitialization
            if self.xmultiservicefactory != None:
                pass

        '''
        Execute Wizard
        @param str only valid parameter is 'start' at the moment.
        '''

        def trigger(self, str):
            if str.equalsIgnoreCase("start"):
                lw = FaxWizardDialogImpl.FaxWizardDialogImpl_unknown(self.xmultiservicefactory)
                if not FaxWizardDialogImpl.running:
                    lw.startWizard(self.xmultiservicefactory, None)

        '''
        The service name, that must be used to get an instance of this service.
        The service manager, that gives access to all registered services.
        This method is a member of the interface for initializing an object directly
        after its creation.
        @param object This array of arbitrary objects will be passed to the component
        after its creation.
        @throws com.sun.star.uno.Exception Every exception will not be handled, but
        will be passed to the caller.
        '''

        def initialize(self, object):
            pass

        '''
        This method returns an array of all supported service names.
        @return Array of supported service names.
        '''

        def getSupportedServiceNames(self):
            stringSupportedServiceNames = range(1)
            stringSupportedServiceNames[0] = self.__class__.__serviceName
            return (stringSupportedServiceNames)

        '''
        This method returns true, if the given service will be supported by the
        component.
        @param stringService Service name.
        @return True, if the given service name will be supported.
        '''

        def supportsService(self, stringService):
            booleanSupportsService = False
            if stringService.equals(self.__class__.__serviceName):
                booleanSupportsService = True

            return (booleanSupportsService)

        '''
        This method returns an array of bytes, that can be used to unambiguously
        distinguish between two sets of types, e.g. to realise hashing functionality
        when the object is introspected. Two objects that return the same ID also
        have to return the same set of types in getTypes(). If an unique
        implementation Id cannot be provided this method has to return an empty
        sequence. Important: If the object aggregates other objects the ID has to be
        unique for the whole combination of objects.
        @return Array of bytes, in order to distinguish between two sets.
        '''

        def getImplementationId(self):
            byteReturn = []
            try:
                byteReturn = ("" + self.hashCode()).getBytes()
            except Exception, exception:
                traceback.print_exc()

            return (byteReturn)

        '''
        Return the class name of the component.
        @return Class name of the component.
        '''

        def getImplementationName(self):
            return (WizardImplementation.getName())

        '''
        Provides a sequence of all types (usually interface types) provided by the
        object.
        @return Sequence of all types (usually interface types) provided by the
        service.
        '''

        def getTypes(self):
            typeReturn = []
            try:
                #COMMENTED
                #typeReturn = [new Type (XPropertyAccess.class), new Type (XJob.class), new Type (XJobExecutor.class), new Type (XTypeProvider.class), new Type (XServiceInfo.class), new Type (XInitialization.class)]
            except Exception, exception:
                traceback.print_exc()

            return (typeReturn)

