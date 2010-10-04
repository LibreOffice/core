package complex.framework.autosave;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.util.*;

class ConfigHelper
{
    private XMultiServiceFactory m_xSMGR = null;
    private XHierarchicalNameAccess m_xConfig = null;

    //-----------------------------------------------
    public ConfigHelper(XMultiServiceFactory xSMGR       ,
                        String               sConfigPath ,
                        boolean              bReadOnly   )
        throws com.sun.star.uno.Exception
    {
        m_xSMGR = xSMGR;

        XMultiServiceFactory xConfigRoot = UnoRuntime.queryInterface(XMultiServiceFactory.class, m_xSMGR.createInstance("com.sun.star.configuration.ConfigurationProvider"));

        PropertyValue[] lParams = new PropertyValue[1];
        lParams[0] = new PropertyValue();
        lParams[0].Name  = "nodepath";
        lParams[0].Value = sConfigPath;

        Object aConfig;
        if (bReadOnly)
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", lParams);
        }
        else
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", lParams);
        }

        m_xConfig = UnoRuntime.queryInterface(XHierarchicalNameAccess.class, aConfig);

        if (m_xConfig == null)
        {
            throw new com.sun.star.uno.Exception("Could not open configuration \"" + sConfigPath + "\"");
        }
    }

    //-----------------------------------------------
    public Object readRelativeKey(String sRelPath,
                                  String sKey    )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(XPropertySet.class, m_xConfig.getByHierarchicalName(sRelPath));
            return xPath.getPropertyValue(sKey);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }

    //-----------------------------------------------
    public void writeRelativeKey(String sRelPath,
                                 String sKey    ,
                                 Object aValue  )
        throws com.sun.star.container.NoSuchElementException
    {
        try
        {
            XPropertySet xPath = UnoRuntime.queryInterface(XPropertySet.class, m_xConfig.getByHierarchicalName(sRelPath));
            xPath.setPropertyValue(sKey, aValue);
        }
        catch(com.sun.star.uno.Exception ex)
        {
            throw new com.sun.star.container.NoSuchElementException(ex.getMessage());
        }
    }

    //-----------------------------------------------
    public void flush()
    {
        try
        {
            XChangesBatch xBatch = UnoRuntime.queryInterface(XChangesBatch.class, m_xConfig);
            xBatch.commitChanges();
        }
        catch(com.sun.star.uno.Exception ex)
        {}
    }

    //-----------------------------------------------
    public static Object readDirectKey(XMultiServiceFactory xSMGR      ,
                                       String               sConfigFile,
                                       String               sRelPath   ,
                                       String               sKey       )
        throws com.sun.star.uno.Exception
    {
        ConfigHelper aConfig = new ConfigHelper(xSMGR, sConfigFile, true);
        return aConfig.readRelativeKey(sRelPath, sKey);
    }

    //-----------------------------------------------
    public static void writeDirectKey(XMultiServiceFactory xSMGR      ,
                                      String               sConfigFile,
                                      String               sRelPath   ,
                                      String               sKey       ,
                                      Object               aValue     )
        throws com.sun.star.uno.Exception
    {
        ConfigHelper aConfig = new ConfigHelper(xSMGR, sConfigFile, false);
        aConfig.writeRelativeKey(sRelPath, sKey, aValue);
        aConfig.flush();
    }
}
