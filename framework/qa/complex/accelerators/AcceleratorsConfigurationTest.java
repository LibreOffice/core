/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package complex.accelerators;

// imports
import com.sun.star.awt.KeyEvent;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.embed.XStorage;
import com.sun.star.embed.XTransactedObject;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.ui.XAcceleratorConfiguration;
import com.sun.star.ui.XUIConfigurationManager;
import com.sun.star.ui.XUIConfigurationPersistence;
import com.sun.star.ui.XUIConfigurationStorage;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XChangesBatch;

// import complex.accelerators.KeyMapping;


// ---------- junit imports -----------------
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.FileHelper;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;
// ------------------------------------------
//-----------------------------------------------

/** @short todo document me
 */
public class AcceleratorsConfigurationTest
{

    /** points to the global uno service manager. */
    private XMultiServiceFactory m_xSmgr = null;
    /** the accelerator configuration for testing. */
    private XAcceleratorConfiguration m_xGlobalAccelCfg = null;
    private XAcceleratorConfiguration m_xModuleAccelCfg = null;
    private XAcceleratorConfiguration m_xDocumentAccelCfg = null;
    /** XCS/XCU based accelerator configuration. */
    private XNameAccess m_xPrimaryKeys = null;
    private XNameAccess m_xSecondaryKeys = null;

    //-------------------------------------------
    // test environment
    //-----------------------------------------------
    /** @short todo document me
     */
//  public String[] getTestMethodNames()
//  {
//      return new String[]
//          {
//              "checkGlobalAccelCfg",
//              "checkModuleAccelCfg",
//                "checkDocumentAccelCfg"
//          };
//  }
    //-----------------------------------------------
    /** @short Create the environment for following tests.
     */
    @Before
    public void before()
            throws java.lang.Exception
    {
        // get uno service manager from global test environment
        m_xSmgr = getMSF();

        m_xGlobalAccelCfg = UnoRuntime.queryInterface(XAcceleratorConfiguration.class, m_xSmgr.createInstance("com.sun.star.ui.GlobalAcceleratorConfiguration"));
        m_xModuleAccelCfg = UnoRuntime.queryInterface(XAcceleratorConfiguration.class, m_xSmgr.createInstance("com.sun.star.ui.ModuleAcceleratorConfiguration"));
        m_xDocumentAccelCfg = UnoRuntime.queryInterface(XAcceleratorConfiguration.class, m_xSmgr.createInstance("com.sun.star.ui.DocumentAcceleratorConfiguration"));

        String sConfigPath = "org.openoffice.Office.Accelerators";
        boolean bReadOnly = false;
        XNameAccess m_xConfig2 = openConfig(m_xSmgr, sConfigPath, bReadOnly);
        if (m_xConfig2 != null)
        {
            m_xPrimaryKeys = UnoRuntime.queryInterface(XNameAccess.class, m_xConfig2.getByName("PrimaryKeys"));
            m_xSecondaryKeys = UnoRuntime.queryInterface(XNameAccess.class, m_xConfig2.getByName("SecondaryKeys"));
        }
    }

    //-------------------------------------------
    /** @short  close the environment.
     */
    @After
    public void after()
            throws java.lang.Exception
    {
        m_xGlobalAccelCfg = null;
        m_xModuleAccelCfg = null;
        m_xDocumentAccelCfg = null;
        m_xSmgr = null;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    @Test
    public void checkGlobalAccelCfg()
            throws java.lang.Exception
    {
        System.out.println("\n---- check Global accelerator configuration: ----");

        String[] sKeys;
        XNameAccess xPrimaryAccess = UnoRuntime.queryInterface(XNameAccess.class, m_xPrimaryKeys.getByName("Global"));
        XNameAccess xSecondaryAccess = UnoRuntime.queryInterface(XNameAccess.class, m_xSecondaryKeys.getByName("Global"));

        sKeys = new String[]
                {
                    "A_MOD1"
                };
        impl_checkGetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, sKeys);

        sKeys = new String[]
                {
                    "PASTE", "X_SHIFT"
                };
        String[] sCommands = new String[]
        {
            ".uno:test", ".uno:test"
        };
        impl_checkSetKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys, sCommands);

        sKeys = new String[]
                {
                    "C_MOD1", "CUT"
                };
        impl_checkRemoveKeyCommands(m_xGlobalAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

        String[] sCommandList = new String[]
        {
            ".uno:Paste", ".uno:CloseWin"
        };
        impl_checkGetPreferredKeyEventsForCommandList(m_xGlobalAccelCfg, xPrimaryAccess, sCommandList);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    @Test
    public void checkModuleAccelCfg()
            throws java.lang.Exception
    {
        String[] sModules = new String[]
        {
            "com.sun.star.frame.StartModule",
            "com.sun.star.drawing.DrawingDocument",
            "com.sun.star.presentation.PresentationDocument",
            "com.sun.star.sheet.SpreadsheetDocument",
            "com.sun.star.text.TextDocument",
        // add other modules here
        };

        for (int i = 0; i < sModules.length; ++i)
        {
            System.out.println("\n---- check accelerator configuration depending module: " + sModules[i] + " ----");

            PropertyValue[] aProp = new PropertyValue[2];
            aProp[0] = new PropertyValue();
            aProp[0].Name = "ModuleIdentifier";
            aProp[0].Value = sModules[i];
            aProp[1] = new PropertyValue();
            aProp[1].Name = "Locale";
            aProp[1].Value = "en-US";

            XInitialization xInit = UnoRuntime.queryInterface(XInitialization.class, m_xModuleAccelCfg);
            xInit.initialize(aProp); // to fill cache

            XNameAccess xPrimaryModules = UnoRuntime.queryInterface(XNameAccess.class, m_xPrimaryKeys.getByName("Modules"));
            XNameAccess xSecondaryModules = UnoRuntime.queryInterface(XNameAccess.class, m_xSecondaryKeys.getByName("Modules"));

            String[] sKeys;
            XNameAccess xPrimaryAccess = UnoRuntime.queryInterface(XNameAccess.class, xPrimaryModules.getByName(sModules[i]));
            XNameAccess xSecondaryAccess = UnoRuntime.queryInterface(XNameAccess.class, xSecondaryModules.getByName(sModules[i]));

            //--------------------------------------------
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
            {
                sKeys = new String[]
                        {
                            "A_SHIFT_MOD1_MOD2"
                        };
            }
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
            {
                sKeys = new String[]
                        {
                            "B_MOD1"
                        };
            }
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
            {
                sKeys = new String[]
                        {
                            "F11_MOD1"
                        };
            }
            else
            {
                sKeys = new String[]
                        {
                            "A_MOD1"
                        };
            }
            impl_checkGetKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, sKeys);

            //--------------------------------------------
            String[] sCommands;
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
            {
                sKeys = new String[]
                        {
                            "A_SHIFT_MOD1_MOD2"
                        };
                sCommands = new String[]
                        {
                            ".uno:test"
                        };
            }
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
            {
                sKeys = new String[]
                        {
                            "B_MOD1"
                        };
                sCommands = new String[]
                        {
                            ".uno:test"
                        };
            }
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
            {
                sKeys = new String[]
                        {
                            "F11_MOD1"
                        };
                sCommands = new String[]
                        {
                            ".uno:test"
                        };
            }
            else
            {
                sKeys = new String[]
                        {
                            "PASTE"
                        };
                sCommands = new String[]
                        {
                            ".uno:test"
                        };
            }
            impl_checkSetKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys, sCommands);

            //--------------------------------------------
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
            {
                sKeys = new String[]
                        {
                            "A_SHIFT_MOD1_MOD2"
                        };
            }
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
            {
                sKeys = new String[]
                        {
                            "F5_SHIFT_MOD1"
                        };
            }
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
            {
                sKeys = new String[]
                        {
                            "BACKSPACE_MOD2"
                        };
            }
            else
            {
                sKeys = new String[]
                        {
                            "C_MOD1"
                        };
            }
            impl_checkRemoveKeyCommands(m_xModuleAccelCfg, xPrimaryAccess, xSecondaryAccess, sKeys);

            //--------------------------------------------
            String[] sCommandList;
            if (sModules[i].equals("com.sun.star.presentation.PresentationDocument"))
            {
                sCommandList = new String[]
                        {
                            ".uno:Presentation"
                        };
            }
            else if (sModules[i].equals("com.sun.star.sheet.SpreadsheetDocument"))
            {
                sCommandList = new String[]
                        {
                            ".uno:InsertCell"
                        };
            }
            else if (sModules[i].equals("com.sun.star.text.TextDocument"))
            {
                sCommandList = new String[]
                        {
                            ".uno:SelectionModeBlock"
                        };
            }
            else
            {
                sCommandList = new String[]
                        {
                            ".uno:Cut"
                        };
            }
            impl_checkGetPreferredKeyEventsForCommandList(m_xModuleAccelCfg, xPrimaryAccess, sCommandList);
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    @Test
    public void checkDocumentAccelCfg()
            throws java.lang.Exception
    {
        System.out.println("\n---- check Document accelerator configuration: ----");

        String sDocCfgName;

        String tempDirURL = util.utils.getOfficeTemp/*Dir*/(getMSF());
        sDocCfgName = FileHelper.appendPath(tempDirURL, "test.cfg");
        // sDocCfgName = "file:///c:/test.cfg";
        SaveDocumentAcceleratorConfiguration(sDocCfgName);

        // sDocCfgName = "file:///c:/test.cfg";
        LoadDocumentAcceleratorConfiguration(sDocCfgName);
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkGetKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xAccess, String[] sKeys)
            throws java.lang.Exception
    {
        System.out.println("check getKeyCommands...");

        for (int i = 0; i < sKeys.length; ++i)
        {
            if (xAccess.hasByName(sKeys[i]) && getCommandFromConfiguration(xAccess, sKeys[i]).length() > 0)
            {
                System.out.println("** get command by " + sKeys[i] + " **");

                String sCmdFromCache = new String(); // get a value using XAcceleratorConfiguration API
                String sCmdFromConfiguration = new String(); // get a value using configuration API

                // GET shortcuts/commands using XAcceleratorConfiguration API
                sCmdFromCache = xAccelCfg.getCommandByKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                System.out.println(sKeys[i] + "-->" + sCmdFromCache + ", by XAcceleratorConfiguration API");

                // GET shortcuts/commands using configuration API
                sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);
                System.out.println(sKeys[i] + "-->" + sCmdFromConfiguration + ", by configuration API");

                assertTrue("values are different by XAcceleratorConfiguration API and configuration API!", sCmdFromCache.equals(sCmdFromConfiguration));

                String sLocale = "es";
                setOfficeLocale(sLocale);
                sCmdFromConfiguration = getCommandFromConfiguration(xAccess, sKeys[i]);
                System.out.println(sKeys[i] + "-->" + sCmdFromConfiguration + ", by configuration API" + " for locale:" + getOfficeLocale());

                sLocale = "en-US";
                setOfficeLocale(sLocale); //reset to default locale
            }
            else
            {
                System.out.println(sKeys[i] + " doesn't exist!");
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkSetKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, XNameAccess xSecondaryAccess, String[] sKeys, String[] sCommands)
            throws java.lang.Exception
    {
        System.out.println("check setKeyCommands...");

        for (int i = 0; i < sKeys.length; ++i)
        {
            if (!xPrimaryAccess.hasByName(sKeys[i]) && !xSecondaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                xAccelCfg.store();
                if (xPrimaryAccess.hasByName(sKeys[i]))
                {
                    System.out.println("add " + sKeys[i] + " successfully!");
                }
                else
                {
                    System.out.println("add " + sKeys[i] + " failed!");
                }
            }
            else if (xPrimaryAccess.hasByName(sKeys[i]))
            {
                String sOriginalCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                if (!sCommands[i].equals(sOriginalCommand))
                {
                    xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                    xAccelCfg.store();

                    String sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                    if (sCommands[i].equals(sChangedCommand))
                    {
                        System.out.println("change " + sKeys[i] + " successfully!");
                    }
                    else
                    {
                        System.out.println("change " + sKeys[i] + " failed!");
                    }
                }
                else
                {
                    System.out.println(sKeys[i] + " already exist!");
                }
            }
            else if (xSecondaryAccess.hasByName(sKeys[i]))
            {
                String sOriginalCommand = getCommandFromConfiguration(xSecondaryAccess, sKeys[i]);
                if (!sCommands[i].equals(sOriginalCommand))
                {
                    xAccelCfg.setKeyEvent(convertShortcut2AWTKey(sKeys[i]), sCommands[i]);
                    xAccelCfg.store();

                    String sChangedCommand = getCommandFromConfiguration(xPrimaryAccess, sKeys[i]);
                    if (sCommands[i].equals(sChangedCommand))
                    {
                        System.out.println("change " + sKeys[i] + " successfully!");
                    }
                    else
                    {
                        System.out.println("change " + sKeys[i] + " failed!");
                    }
                }
                else
                {
                    System.out.println(sKeys[i] + " already exist!");
                }
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkRemoveKeyCommands(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, XNameAccess xSecondaryAccess, String[] sKeys)
            throws java.lang.Exception
    {
        System.out.println("check removeKeyCommands...");

        for (int i = 0; i < sKeys.length; i++)
        {
            if (!xPrimaryAccess.hasByName(sKeys[i]) && !xSecondaryAccess.hasByName(sKeys[i]))
            {
                System.out.println(sKeys[i] + " doesn't exist!");
            }
            else if (xPrimaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                xAccelCfg.store();
                if (!xPrimaryAccess.hasByName(sKeys[i]))
                {
                    System.out.println("Remove " + sKeys[i] + " successfully!");
                }
                else
                {
                    System.out.println("Remove " + sKeys[i] + " failed!");
                }
            }
            else if (xSecondaryAccess.hasByName(sKeys[i]))
            {
                xAccelCfg.removeKeyEvent(convertShortcut2AWTKey(sKeys[i]));
                xAccelCfg.store();
                if (!xSecondaryAccess.hasByName(sKeys[i]))
                {
                    System.out.println("Remove " + sKeys[i] + " successfully!");
                }
                else
                {
                    System.out.println("Remove " + sKeys[i] + " failed!");
                }
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void impl_checkGetPreferredKeyEventsForCommandList(XAcceleratorConfiguration xAccelCfg, XNameAccess xPrimaryAccess, String[] sCommandList)
            throws java.lang.Exception
    {
        System.out.println("check getPreferredKeyEventsForCommandList...");

        Object[] oKeyEvents = xAccelCfg.getPreferredKeyEventsForCommandList(sCommandList);
        for (int i = 0; i < oKeyEvents.length; i++)
        {
            System.out.println("get preferred key for command " + sCommandList[i] + ":");

            KeyEvent aKeyEvent = (KeyEvent) AnyConverter.toObject(KeyEvent.class, oKeyEvents[i]);
            String sKeyEvent = convertAWTKey2Shortcut(aKeyEvent);
            System.out.println(sKeyEvent);

            String sCmdFromConfiguration = getCommandFromConfiguration(xPrimaryAccess, sKeyEvent);
            System.out.println(sCmdFromConfiguration);
            if (sCommandList[i].equals(sCmdFromConfiguration))
            {
                System.out.println("get preferred key correctly!");
            }
            else
            {
                System.out.println("get preferred key failed!");
            }
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String getCommandFromConfiguration(XNameAccess xAccess, String sKey)
            throws java.lang.Exception
    {
        String sCommand = new String();

        if (xAccess.hasByName(sKey))
        {
            XNameAccess xKey = UnoRuntime.queryInterface(XNameAccess.class, xAccess.getByName(sKey));
            XNameAccess xCommand = UnoRuntime.queryInterface(XNameAccess.class, xKey.getByName("Command"));

            String sLocale = getOfficeLocale();
            if (xCommand.hasByName(sLocale))
            {
                sCommand = UnoRuntime.queryInterface(String.class, xCommand.getByName(sLocale));
            }
        }

        return sCommand;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void LoadDocumentAcceleratorConfiguration(String sDocCfgName)
            throws java.lang.Exception
    {
        XSingleServiceFactory xStorageFactory = UnoRuntime.queryInterface(XSingleServiceFactory.class, m_xSmgr.createInstance("com.sun.star.embed.StorageFactory"));

        Object aArgs[] = new Object[2];
        aArgs[0] = sDocCfgName;
        aArgs[1] = new Integer(com.sun.star.embed.ElementModes.READ);
        XStorage xRootStorage = UnoRuntime.queryInterface(XStorage.class, xStorageFactory.createInstanceWithArguments(aArgs));

        XStorage xUIConfig = xRootStorage.openStorageElement("Configurations2", com.sun.star.embed.ElementModes.READ);

        PropertyValue aProp = new PropertyValue();
        aProp.Name = "DocumentRoot";
        aProp.Value = xUIConfig;
        Object[] lArgs = new Object[1];
        lArgs[0] = aProp;

        XInitialization xInit = UnoRuntime.queryInterface(XInitialization.class, m_xDocumentAccelCfg);
        xInit.initialize(lArgs);

        // TODO: throws css::container::NoSuchElementException
        try
        {
            String test = m_xDocumentAccelCfg.getCommandByKeyEvent(convertShortcut2AWTKey("F2"));
            System.out.println(test);
        }
        catch(com.sun.star.container.NoSuchElementException e)
        {
            System.out.println("NoSuchElementException caught: " + e.getMessage());
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void SaveDocumentAcceleratorConfiguration(String sDocCfgName)
            throws java.lang.Exception
    {
        XSingleServiceFactory xStorageFactory = UnoRuntime.queryInterface(XSingleServiceFactory.class, m_xSmgr.createInstance("com.sun.star.embed.StorageFactory"));

        Object aArgs[] = new Object[2];
        aArgs[0] = sDocCfgName;
        aArgs[1] = new Integer(com.sun.star.embed.ElementModes.WRITE);
        XStorage xRootStorage = UnoRuntime.queryInterface(XStorage.class, xStorageFactory.createInstanceWithArguments(aArgs));

        XStorage xUIConfig = xRootStorage.openStorageElement("Configurations2", com.sun.star.embed.ElementModes.WRITE);

        XUIConfigurationManager xCfgMgr = UnoRuntime.queryInterface(XUIConfigurationManager.class, m_xSmgr.createInstance("com.sun.star.ui.UIConfigurationManager"));

        XUIConfigurationStorage xUICfgStore = UnoRuntime.queryInterface(XUIConfigurationStorage.class, xCfgMgr);
        xUICfgStore.setStorage(xUIConfig);

        XPropertySet xUIConfigProps = UnoRuntime.queryInterface(XPropertySet.class, xUIConfig);
        xUIConfigProps.setPropertyValue("MediaType", "application/vnd.sun.xml.ui.configuration");

        if (xCfgMgr != null)
        {
            XAcceleratorConfiguration xTargetAccMgr = UnoRuntime.queryInterface(XAcceleratorConfiguration.class, xCfgMgr.getShortCutManager());
            XUIConfigurationPersistence xCommit1 = UnoRuntime.queryInterface(XUIConfigurationPersistence.class, xTargetAccMgr);
            XUIConfigurationPersistence xCommit2 = UnoRuntime.queryInterface(XUIConfigurationPersistence.class, xCfgMgr);
            xCommit1.store();
            xCommit2.store();

            XTransactedObject xCommit3 = UnoRuntime.queryInterface(XTransactedObject.class, xRootStorage);
            xCommit3.commit();
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private com.sun.star.awt.KeyEvent convertShortcut2AWTKey(String sShortcut)
            throws java.lang.Exception
    {
        com.sun.star.awt.KeyEvent aKeyEvent = new com.sun.star.awt.KeyEvent();
        KeyMapping aKeyMapping = new KeyMapping();
        String[] sShortcutSplits = sShortcut.split("_");

        aKeyEvent.KeyCode = aKeyMapping.mapIdentifier2Code(sShortcutSplits[0]);
        for (int i = 1; i < sShortcutSplits.length; i++)
        {
            if (sShortcutSplits[i].equals("SHIFT"))
            {
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.SHIFT;
            }
            else if (sShortcutSplits[i].equals("MOD1"))
            {
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.MOD1;
            }
            else if (sShortcutSplits[i].equals("MOD2"))
            {
                aKeyEvent.Modifiers |= com.sun.star.awt.KeyModifier.MOD2;
            }
        }

        return aKeyEvent;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String convertAWTKey2Shortcut(com.sun.star.awt.KeyEvent aKeyEvent)
            throws java.lang.Exception
    {
        String sShortcut;

        KeyMapping aKeyMapping = new KeyMapping();
        sShortcut = aKeyMapping.mapCode2Identifier(aKeyEvent.KeyCode);

        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.SHIFT) == com.sun.star.awt.KeyModifier.SHIFT)
        {
            sShortcut += "_SHIFT";
        }
        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.MOD1) == com.sun.star.awt.KeyModifier.MOD1)
        {
            sShortcut += "_MOD1";
        }
        if ((aKeyEvent.Modifiers & com.sun.star.awt.KeyModifier.MOD2) == com.sun.star.awt.KeyModifier.MOD2)
        {
            sShortcut += "_MOD2";
        }

        return sShortcut;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private String getOfficeLocale()
            throws java.lang.Exception
    {
        String sLocale = new String();

        String sConfigPath = "org.openoffice.Setup";
        boolean bReadOnly = true;
        XNameAccess xRootConfig = openConfig(m_xSmgr, sConfigPath, bReadOnly);

        if (xRootConfig != null)
        {
            XNameAccess xLocale = UnoRuntime.queryInterface(XNameAccess.class, xRootConfig.getByName("L10N"));
            XPropertySet xSet = UnoRuntime.queryInterface(XPropertySet.class, xLocale);
            sLocale = (String) xSet.getPropertyValue("ooLocale");
        }

        return sLocale;
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private void setOfficeLocale(String sLocale)
            throws java.lang.Exception
    {
        String sConfigPath = "org.openoffice.Setup";
        boolean bReadOnly = false;
        XNameAccess xRootConfig = openConfig(m_xSmgr, sConfigPath, bReadOnly);

        if (xRootConfig != null)
        {
            XNameAccess xLocale = UnoRuntime.queryInterface(XNameAccess.class, xRootConfig.getByName("L10N"));
            XPropertySet xSet = UnoRuntime.queryInterface(XPropertySet.class, xLocale);
            xSet.setPropertyValue("ooLocale", sLocale);
            XChangesBatch xBatch = UnoRuntime.queryInterface(XChangesBatch.class, xRootConfig);
            xBatch.commitChanges();
        }
    }

    //-------------------------------------------
    /** @todo document me.
     */
    private XNameAccess openConfig(XMultiServiceFactory xSMGR,
            String sConfigPath,
            boolean bReadOnly)
            throws java.lang.Exception
    {
        XMultiServiceFactory xConfigRoot = UnoRuntime.queryInterface(XMultiServiceFactory.class, xSMGR.createInstance("com.sun.star.configuration.ConfigurationProvider"));

        PropertyValue[] lParams = new PropertyValue[2];
        lParams[0] = new PropertyValue();
        lParams[0].Name = "nodepath";
        lParams[0].Value = sConfigPath;

        lParams[1] = new PropertyValue();
        lParams[1].Name = "locale";
        lParams[1].Value = "*";

        Object aConfig;
        if (bReadOnly)
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", lParams);
        }
        else
        {
            aConfig = xConfigRoot.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", lParams);
        }

        XNameAccess xConfig = UnoRuntime.queryInterface(XNameAccess.class, aConfig);

        if (xConfig == null)
        {
            throw new com.sun.star.uno.Exception("Could not open configuration \"" + sConfigPath + "\"");
        }

        return xConfig;
    }

    private XMultiServiceFactory getMSF()
    {
        return(UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager()));
    }

    // setup and close connections
    @BeforeClass
    public static void setUpConnection() throws Exception
    {
        System.out.println("setUpConnection()");
        connection.setUp();
    }

    @AfterClass
    public static void tearDownConnection()
            throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println("tearDownConnection()");
        connection.tearDown();
    }
    private static final OfficeConnection connection = new OfficeConnection();
}
