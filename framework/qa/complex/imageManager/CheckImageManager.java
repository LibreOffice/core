package imageManager;

import imageManager.interfaces._XComponent;
import imageManager.interfaces._XImageManager;
import imageManager.interfaces._XInitialization;
import imageManager.interfaces._XTypeProvider;
import imageManager.interfaces._XUIConfiguration;
import imageManager.interfaces._XUIConfigurationPersistence;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import drafts.com.sun.star.ui.XImageManager;
import complexlib.ComplexTestCase;
import drafts.com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import drafts.com.sun.star.ui.XUIConfiguration;
import drafts.com.sun.star.ui.XUIConfigurationManager;
import drafts.com.sun.star.ui.XUIConfigurationPersistence;
import java.io.PrintWriter;
import share.LogWriter;

/**
 *
 */
public class CheckImageManager extends ComplexTestCase {
    boolean checkUIConfigManager = false;
    XMultiServiceFactory xMSF = null;

    public void before() {
        xMSF = (XMultiServiceFactory)param.getMSF();
    }

    public String[] getTestMethodNames() {
        return new String[]{"checkImageManagerFromModule"};//, "checkImageManager"};
    }

    public void checkImageManagerFromModule() {
        log.println(" **** ImageManager from ModuleUIConfigurationManager *** ");
        XUIConfigurationManager xManager = null;
        try {
            Object o = (XInterface)xMSF.createInstance(
                    "drafts.com.sun.star.ui.ModuleUIConfigurationManagerSupplier");
            XModuleUIConfigurationManagerSupplier xMUICMS =
                    (XModuleUIConfigurationManagerSupplier)UnoRuntime.queryInterface(
                    XModuleUIConfigurationManagerSupplier.class, o);
            xManager = xMUICMS.getUIConfigurationManager(
                    "com.sun.star.text.TextDocument");
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((PrintWriter)log);
            failed("Exception. " + e.getMessage());
        }
        XImageManager xImageManager = (XImageManager)UnoRuntime.queryInterface(
                XImageManager.class, xManager.getImageManager());
        performChecks(xImageManager, "ModuleUIConfig", xManager);
    }

    public void checkImageManager() {
        log.println(" **** ImageManager from UIConfigurationManager *** ");
        XUIConfigurationManager xManager = null;
        try {
            xManager = (XUIConfigurationManager)UnoRuntime.queryInterface(
                    XUIConfigurationManager.class, xMSF.createInstance(
                    "com.sun.star.comp.framework.UIConfigurationManager"));
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((PrintWriter)log);
            failed("Exception. " + e.getMessage());
        }

        XImageManager xImageManager = (XImageManager)UnoRuntime.queryInterface(
                                XImageManager.class, xManager.getImageManager());
        performChecks(xImageManager, "UIConfig", xManager);
    }

    private void performChecks(XImageManager xImageManager, String testObjectName, XUIConfigurationManager xManager) {
        util.dbg.printInterfaces(xImageManager);

        OXUIConfigurationListenerImpl configListener = new OXUIConfigurationListenerImpl(log, xManager, xMSF);
        param.put("XUIConfiguration.XUIConfigurationListenerImpl", configListener);

        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(XInitialization.class, xImageManager);
        _XInitialization _xInit = new _XInitialization(log, param, xInit);
        assure(testObjectName + "::XInitialization.initialize", _xInit._initialize(), true);

        // xImageManager is already there, just write a test ;-)
        _XImageManager _xImage = new _XImageManager(log, param, xImageManager);
        assure(testObjectName + "::XImageManager.getAllImageNames", _xImage._getAllImageNames(), true);
        assure(testObjectName + "::XImageManager.getImages", _xImage._getImages(), true);
        assure(testObjectName + "::XImageManager.hasImage", _xImage._hasImage(), true);
        assure(testObjectName + "::XImageManager.insertImages", _xImage._insertImages(), true);
        assure(testObjectName + "::XImageManager.removeImages", _xImage._removeImages(), true);
        assure(testObjectName + "::XImageManager.replaceImages", _xImage._replaceImages(), true);
        assure(testObjectName + "::XImageManager.reset", _xImage._reset(), true);

        XTypeProvider xType = (XTypeProvider)UnoRuntime.queryInterface(XTypeProvider.class, xImageManager);
        _XTypeProvider _xType = new _XTypeProvider(log,param,xType);
        assure(testObjectName + "::XTypeProvider.getImplementationId", _xType._getImplementationId(), true);
        assure(testObjectName + "::XTypeProvider.getTypes", _xType._getTypes(), true);

        XUIConfiguration xUIConfig = (XUIConfiguration)UnoRuntime.queryInterface(XUIConfiguration.class, xImageManager);
        _XUIConfiguration _xUIConfig = new _XUIConfiguration(log,  param, xUIConfig);
        _xUIConfig.before();
        assure(testObjectName + "::XUIConfig.addConfigurationListener", _xUIConfig._addConfigurationListener(), true);
        assure(testObjectName + "::XUIConfig.removeConfigurationListener", _xUIConfig._removeConfigurationListener(), true);

        XUIConfigurationPersistence xUIConfigPersistence = (XUIConfigurationPersistence)UnoRuntime.queryInterface(XUIConfiguration.class, xImageManager);
        _XUIConfigurationPersistence _xUIConfigPersistence = new _XUIConfigurationPersistence(log, param, xUIConfigPersistence);
        _xUIConfigPersistence.before();
        assure(testObjectName + "::XUIConfigPersistence.isModified", _xUIConfigPersistence._isModified(), true);
        assure(testObjectName + "::XUIConfigPersistence.isReadOnly", _xUIConfigPersistence._isReadOnly(), true);
        assure(testObjectName + "::XUIConfigPersistence.reload", _xUIConfigPersistence._reload(), true);
        assure(testObjectName + "::XUIConfigPersistence.store", _xUIConfigPersistence._store(), true);
        assure(testObjectName + "::XUIConfigPersistence.storeToStorage", _xUIConfigPersistence._storeToStorage(), true);

        XComponent xComp = (XComponent)UnoRuntime.queryInterface(XComponent.class, xImageManager);
        _XComponent _xComp = new _XComponent(log, param, xComp);
        _xComp.before();
        assure(testObjectName + "::XComponent.addEventListener", _xComp._addEventListener(), true);
        assure(testObjectName + "::XComponent.removeEventListener", _xComp._removeEventListener(), true);
        assure(testObjectName + "::XComponent.dispose", _xComp._dispose(), true);
    }


    class OXUIConfigurationListenerImpl implements _XUIConfiguration.XUIConfigurationListenerImpl {
        private boolean triggered = false;
        private LogWriter log = null;
        private XUIConfigurationManager xUIManager = null;
        private XMultiServiceFactory xMSF = null;

        public OXUIConfigurationListenerImpl(LogWriter _log, XUIConfigurationManager xUIManager, XMultiServiceFactory xMSF) {
            log = _log;
            this.xUIManager = xUIManager;
            this.xMSF = xMSF;
        }

        public boolean actionWasTriggered() {
            return triggered;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
            triggered = true;
        }

        public void elementInserted(drafts.com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }

        public void elementRemoved(drafts.com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }

        public void elementReplaced(drafts.com.sun.star.ui.ConfigurationEvent configurationEvent) {
            triggered = true;
        }

        public void fireEvent() {
            // remove for real action:
            triggered = !triggered;
/*            try {
                XIndexAccess xMenuBarSettings = xUIManager.getSettings(
                                        "private:resource/menubar/menubar", true);

            }
            catch(com.sun.star.container.NoSuchElementException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IllegalArgumentException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IllegalAccessException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                log.println("_XUIConfiguration.XUIConfigurationListenerImpl: Exception.");
                e.printStackTrace((java.io.PrintWriter)log);
            } */
        }

        public void reset() {
            // remove comment for real function
            //triggered = false;
        }

    }
}
