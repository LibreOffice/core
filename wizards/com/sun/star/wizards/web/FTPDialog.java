/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.web;

import java.util.List;
import java.util.Vector;

import com.sun.star.awt.PushButtonType;
import com.sun.star.awt.XButton;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XTextComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.AuthenticationRequest;
import com.sun.star.ucb.InteractiveAugmentedIOException;
import com.sun.star.ucb.InteractiveNetworkConnectException;
import com.sun.star.ucb.InteractiveNetworkResolveNameException;
import com.sun.star.ucb.OpenCommandArgument2;
import com.sun.star.ucb.OpenMode;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.UCB;
import com.sun.star.wizards.ui.UIConsts;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.event.DataAware;
import com.sun.star.wizards.ui.event.UnoDataAware;
import com.sun.star.wizards.web.data.CGPublish;
import com.sun.star.wizards.web.data.CGSettings;
import com.sun.star.wizards.common.HelpIds;

/**
 * This is the FTP Dialog. <br/>
 * The Dialog enables the user:
 * (*) entering FTP server and user information.
 * (*) testing the connection.
 * (*) choosing a directory on the server.
 * If a connection was established succesfully, the user may
 * press OK, which will change
 * the CGPublish object propertiers according the user's input.
 * If no connection was established. the OK and Choose-Dir button are disabled.
 * See the method "disconnect()" which disables them.
 *
 * I use here the DataAware concept to automatically update
 * the members ip, username, and password (via the methods setXXX(...))
 * for details see the ui.events.DataAware classes. <br/>
 */
public class FTPDialog extends UnoDialog2 implements UIConsts, WWHID
{

    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display. "unknown" is the status when the user first
     * opens the dialog, or changes the servername/username/password.
     */
    private final static int STATUS_UNKONWN = 0;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display. (connection established)
     */
    private final static int STATUS_OK = 1;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display.
     */
    private final static int STATUS_USER_PWD_WRONG = 2;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display.
     */
    private final static int STATUS_SERVER_NOT_FOUND = 3;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display.
     */
    private final static int STATUS_NO_RIGHTS = 4;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display.
     */
    private final static int STATUS_HOST_UNREACHABLE = 5;
    /**
     * A Constant used for the setLabel(int) method to change the
     * status-display.
     */
    private final static int STATUS_CONNECTING = 6;
    /**
     * The icon url for error
     */
    private final static String ICON_ERROR = "ftperror.gif";
    /**
     * The icon url for ok (connection ok)
     */
    private final static String ICON_OK = "ftpconnected.gif";
    /**
     * The icon url for unknown - this is the status when
     * the user first opens the dialog
     */
    private final static String ICON_UNKNOWN = "ftpunknown.gif";
    /**
     * The icon url for an icon representing the "connecting" state.
     */
    private final static String ICON_CONNECTING = "ftpconnecting.gif";    //GUI Components as Class members.
    //Fixed Line
    private XControl ln1;
    private XFixedText lblFTPAddress;
    private XTextComponent txtHost;
    private XFixedText lblUsername;
    private XTextComponent txtUsername;
    private XFixedText lblPassword;
    private XTextComponent txtPassword;
    //Fixed Line
    private XControl ln2;
    private XButton btnTestConnection;
    private XControl imgStatus;
    private XFixedText lblStatus;
    //Fixed Line
    private XControl ln3;
    private XTextComponent txtDir;
    private XButton btnDir;
    private XButton btnOK;
    private XButton btnCancel;
    private XButton btnHelp;
    //Font Descriptors as Class members.

    //Resources Object
    private FTPDialogResources resources;
    private List dataAware = new Vector();
    public String username = "";
    public String password = "";
    /**
     * The ftp host name
     */
    public String host = "";
    /**
     * The ftp directory.
     */
    private String dir = "";
    /**
     * the ftp publish object which contains the
     * data for this dialog.
     */
    private CGPublish publish;
    private UCB ucb;
    /**
     * used for the status images url.
     */
    private String imagesDirectory;

    /**
     * constructor.
     * constructs the UI.
     * @param xmsf
     * @param p the publishert object that contains the data
     * for this dialog
     * @throws Exception
     */
    public FTPDialog(XMultiServiceFactory xmsf, CGPublish p) throws Exception
    {
        super(xmsf);
        publish = p;


        imagesDirectory = FileAccess.connectURLs(((CGSettings) (publish.root)).soTemplateDir, "../wizard/bitmap/");

        //Load Resources
        resources = new FTPDialogResources(xmsf);
        ucb = new UCB(xmsf);

        //set dialog properties...
        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    PropertyNames.PROPERTY_CLOSEABLE, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.TRUE, 160, HelpIds.getHelpIdString(HID_FTP), Boolean.TRUE, "FTPDialog", 167, 82, resources.resFTPDialog_title, 222
                });

        //add controls to dialog
        build();
        //make the hostname, username and password textfield data-aware.
        configure();
        //make sure we display a disconnected status.
        disconnect();
    }

    /**
     * Add controls to dialog.
     */
    public void build()
    {
        final String[] PROPNAMES_LABEL = new String[]
        {
            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
        };
        final String[] PROPNAMES_BUTTON = new String[]
        {
            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
        };
        final String[] PROPNAMES_BUTTON2 = new String[]
        {
            PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "PushButtonType", PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
        };

        ln1 = insertFixedLine("ln1",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.resln1_value, "ln1", INTEGERS[6], INTEGERS[6], new Short((short) 0), 210
                });
        lblFTPAddress = insertLabel("lblFTPAddress",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblFTPAddress_value, "lblFTPAddress", INTEGER_12, 20, new Short((short) 1), 95
                });
        txtHost = insertTextField("txtHost", "disconnect",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID_FTP_SERVER), "txtIP", 110, 18, new Short((short) 2), 106
                });
        lblUsername = insertLabel("lblUsername",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblUsername_value, "lblUsername", INTEGER_12, 36, new Short((short) 3), 85
                });
        txtUsername = insertTextField("txtUsername", "disconnect",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    INTEGER_12, HelpIds.getHelpIdString(HID_FTP_USERNAME), "txtUsername", 110, 34, new Short((short) 4), 106
                });
        lblPassword = insertLabel("lblPassword",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.reslblPassword_value, "lblPassword", INTEGER_12, 52, new Short((short) 5), 85
                });
        txtPassword = insertTextField("txtPassword", "disconnect",
                new String[]
                {
                    "EchoChar", PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 42), INTEGER_12, HelpIds.getHelpIdString(HID_FTP_PASS), "txtPassword", 110, 50, new Short((short) 6), 106
                });
        ln2 = insertFixedLine("ln2",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.resln2_value, "ln2", INTEGERS[6], 68, new Short((short) 7), 210
                });
        btnTestConnection = insertButton("btnConnect", "connect",
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID_FTP_TEST), resources.resbtnConnect_value, "btnConnect", INTEGER_12, 80, new Short((short) 8), INTEGER_50
                });

        imgStatus = insertImage("imgStatus",
                new String[]
                {
                    PropertyNames.PROPERTY_BORDER, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, "ScaleImage", "Tabstop", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Short((short) 0), INTEGER_14, 68, 80, Boolean.FALSE, Boolean.FALSE, INTEGER_14
                });

        lblStatus = insertLabel("lblStatus",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.resFTPDisconnected, "lblStatus", 86, 82, new Short((short) 9), 99
                });

        ln3 = insertFixedLine("ln3",
                PROPNAMES_LABEL,
                new Object[]
                {
                    INTEGERS[8], resources.resln3_value, "ln3", INTEGERS[6], 100, new Short((short) 10), 210
                });

        txtDir = insertTextField("txtDir",
                null, new String[]
                {
                    PropertyNames.PROPERTY_ENABLED, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, "Text", PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    new Boolean(false), INTEGER_12, HelpIds.getHelpIdString(HID_FTP_TXT_PATH), "txtDir", INTEGER_12, 113, new Short((short) 11), resources.restxtDir_value, 184
                });
        btnDir = insertButton("btnDir", "chooseDirectory",
                PROPNAMES_BUTTON,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID_FTP_BTN_PATH), resources.resbtnDir_value, "btnDir", 199, 112, new Short((short) 12), INTEGER_16
                });

        btnOK = insertButton("btnOK", null,
                PROPNAMES_BUTTON2,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID_FTP_OK), resources.resbtnOK_value, "btnOK", 165, 142, new Short((short) PushButtonType.OK_value), new Short((short) 13), INTEGER_50
                });

        btnCancel = insertButton("btnCancel",
                null, PROPNAMES_BUTTON2,
                new Object[]
                {
                    INTEGER_14, HelpIds.getHelpIdString(HID_FTP_CANCEL), resources.resbtnCancel_value, "btnCancel", 113, 142, new Short((short) PushButtonType.CANCEL_value), new Short((short) 14), INTEGER_50
                });

        btnHelp = insertButton("btnHelp", null,
                PROPNAMES_BUTTON2,
                new Object[]
                {
                    INTEGER_14, "", resources.resbtnHelp_value, "btnHelp", 57, 142, new Short((short) PushButtonType.HELP_value), new Short((short) 15), INTEGER_50
                });

    }

    /**
     * Make hostname, username and password text fields data aware.
     */
    private void configure()
    {
        dataAware.add(UnoDataAware.attachEditControl(this, "host", txtHost, null, true));
        dataAware.add(UnoDataAware.attachEditControl(this, "username", txtUsername, null, true));
        dataAware.add(UnoDataAware.attachEditControl(this, "password", txtPassword, null, true));
    }

    /**
     * Shows the dialog.
     * If the user clicks ok, changes the given CGPublish properties to the
     * user input.
     * @param parent a dialog to center this dialog to.
     * @return 0 for cancel, 1 for ok.
     * @throws Exception - well, if something goes wrong...
     */
    public short execute(UnoDialog parent) throws Exception
    {
        host = extractHost(publish.cp_URL);
        username = publish.cp_Username == null ? "" : publish.cp_Username;
        password = publish.password == null ? "" : publish.password;
        dir = extractDir(publish.cp_URL);
        setLabel(STATUS_UNKONWN);

        enableTestButton();
        updateUI();
        short result = executeDialog(parent);
        //change the CGPublish properties
        if (result == 1)
        {
            publish.cp_URL = "ftp://" + host() + getDir();
            publish.cp_Username = username;
            publish.password = password;
        }

        return result;
    }

    /**
     * updates the hostname, username, password and
     * directory text fields.
     * is called uppon initialization.
     */
    private void updateUI()
    {
        DataAware.updateUI(dataAware);
        setDir(dir);
    }

    /**
     * extract the hostname out of the url used by the
     * publisher. This url does not include the username:password string.
     * @param ftpUrl
     * @return
     */
    private String extractHost(String ftpUrl)
    {
        if (ftpUrl == null || ftpUrl.length() < 6)
        {
            return "";
        }
        String url = ftpUrl.substring(6);
        int i = url.indexOf("/");
        if (i == -1)
        {
            return url;
        }
        else
        {
            return url.substring(0, i);
        }
    }

    /**
     * used to get data from the CGPublish object.
     * @param ftpUrl
     * @return the directory portion of the ftp-url
     */
    private String extractDir(String ftpUrl)
    {
        if (ftpUrl == null || ftpUrl.length() < 6)
        {
            return "/";
        }
        String url = ftpUrl.substring(6);
        int i = url.indexOf("/");
        if (i == -1)
        {
            return "/";
        }
        else
        {
            return url.substring(i);
        }
    }

    /**
     * enables/disables the "test" button
     * according to the status of the hostname, username, password text fields.
     * If one of these fields is empty, the button is disabled.
     */
    private void enableTestButton()
    {
        setEnabled(btnTestConnection, !(isEmpty(host) || isEmpty(username) || isEmpty(password)));

    }

    /**
     * @param s
     * @return true if the string is null or "".
     */
    private final boolean isEmpty(String s)
    {
        return (s == null) || (s.equals(""));
    }

    /**
     * @return the ftp url with username and password,
     * but without the directory portion.
     */
    public String getAcountUrl()
    {
        return "ftp://" + username + ":" + password + "@" + host();
    }

    /**
     * return the host name without the "ftp://"
     * @return
     */
    private String host()
    {
        return host(host);
    }

    private static String host(String s)
    {
        return (s.startsWith("ftp://") ? s.substring(6) : s);
    }

    /**
     * @return the full ftp url including username, password and directory portion.
     */
    private String getFullUrl()
    {
        return getAcountUrl() + dir;
    }

    /**
     * First I try to connect to the full url, including directory.
     * If an InteractiveAugmentedIOException accures, I try again,
     * this time without the dir spec. If this works, I change the dir
     * to "/", if not I say to the user its his problem...
     *
     */
    public void connect()
    {
        setEnabled(btnTestConnection, false);
        setLabel(STATUS_CONNECTING);
        boolean success = false;
        try
        {
            connect(getFullUrl());
            success = true;
        }
        catch (InteractiveAugmentedIOException iaioex)
        {
            try
            {
                connect(getAcountUrl());
                setDir("/");
                success = true;
            }
            catch (Exception ex)
            {
                setLabel(STATUS_NO_RIGHTS);
            }
        }
        catch (InteractiveNetworkResolveNameException inrne)
        {
            setLabel(STATUS_SERVER_NOT_FOUND);
        }
        catch (AuthenticationRequest ar)
        {
            setLabel(STATUS_USER_PWD_WRONG);
        }
        catch (InteractiveNetworkConnectException incx)
        {
            setLabel(STATUS_HOST_UNREACHABLE);
        }
        catch (Exception ex)
        {
            setLabel(-1);
            ex.printStackTrace();
        }


        if (success)
        {
            setLabel(STATUS_OK);
            setEnabled(btnDir, true);
            setEnabled(btnOK, true);
        }

        setEnabled(btnTestConnection, true);

    }

    /**
     * To try the conenction I do some actions that
     * seem logical to me: <br/>
     * I get a ucb content.
     * I list the files in this content.
     * I call the ucb "open" command.
     * I get the PropertyNames.PROPERTY_TITLE property of this content.
     * @param acountUrl
     * @throws Exception
     */
    private void connect(String acountUrl) throws Exception
    {
        Object content = ucb.getContent(acountUrl);

        //list files in the content.
        List l = ucb.listFiles(acountUrl, null);

        //open the content
        OpenCommandArgument2 aArg = new OpenCommandArgument2();
        aArg.Mode = OpenMode.FOLDERS; // FOLDER, DOCUMENTS -> simple filter
        aArg.Priority = 32768; // Ignored by most implementations

        ucb.executeCommand(content, "open", aArg);

        //get the title property of the content.
        Object obj = ucb.getContentProperty(content, PropertyNames.PROPERTY_TITLE, String.class);

    }

    /**
     * changes the ftp subdirectory, in both
     * the UI and the data.
     * @param s the directory.
     */
    public void setDir(String s)
    {
        dir = s;
        Helper.setUnoPropertyValue(getModel(txtDir), "Text", dir);
    }

    /**
     * @return the ftp subdirecrtory.
     */
    public String getDir()
    {
        return dir;
    }

    /**
     * changes the status label to disconnected status, and
     * disables the ok and choose-dir buttons.
     * This method is called also when the hostname, username
     * and passwordtext fields change.
     */
    public void disconnect()
    {
        enableTestButton();
        setEnabled(btnOK, false);
        setEnabled(btnDir, false);
        setLabel(STATUS_UNKONWN);
    }

    /**
     * used for debuging.
     * @param args
     */
    public static void main(String args[])
    {
        String ConnectStr = "uno:socket,host=localhost,port=8100;urp,negotiate=0,forcesynchronous=1;StarOffice.ServiceManager";
        try
        {
            XMultiServiceFactory xLocMSF = Desktop.connect(ConnectStr);
            CGPublish p = new CGPublish();
            p.cp_URL = "ftp://tv-1/Folder";
            p.cp_Username = "ronftp";
            p.password = "ronftp";

            FTPDialog dialog = new FTPDialog(xLocMSF, p);
            dialog.execute(null);
        }
        catch (Exception exception)
        {
            exception.printStackTrace();
        }
    }

    /**
     * changes the status label and icon, according to the
     * given status
     * @param status one opf the private status-constants.
     * if this param is not one of them, an "unknown error" status is displayed.
     */
    private void setLabel(int status)
    {
        switch (status)
        {
            //not connected yet
            case STATUS_UNKONWN:
                setLabel(resources.resFTPDisconnected, ICON_UNKNOWN);
                break;
            //connected!
            case STATUS_OK:
                setLabel(resources.resFTPConnected, ICON_OK);
                break;
            case STATUS_USER_PWD_WRONG:
                setLabel(resources.resFTPUserPwdWrong, ICON_ERROR);
                break;
            //problem resolving server name
            case STATUS_SERVER_NOT_FOUND:
                setLabel(resources.resFTPServerNotFound, ICON_ERROR);
                break;
            //rights problem
            case STATUS_NO_RIGHTS:
                setLabel(resources.resFTPRights, ICON_ERROR);
                break;
            //host unreachable (firewall?)
            case STATUS_HOST_UNREACHABLE:
                setLabel(resources.resFTPHostUnreachable, ICON_ERROR);
                break;
            case STATUS_CONNECTING:
                setLabel(resources.resConnecting, ICON_CONNECTING);
                break;
            default:
                setLabel(resources.resFTPUnknownError, ICON_ERROR);
        }
    }

    /**
     * changes the text of the status label and
     * (TODO) the status image.
     * @param label
     * @param color
     */
    private void setLabel(String label, String image)
    {
        Helper.setUnoPropertyValue(getModel(lblStatus), PropertyNames.PROPERTY_LABEL, label);
        Helper.setUnoPropertyValue(getModel(imgStatus), PropertyNames.PROPERTY_IMAGEURL, imageUrl(image));
    }

    private String imageUrl(String s)
    {
        String t = imagesDirectory + s;
        //System.out.println(t);
        return t;

    }

    /**
     * called when the user clicks
     * the choose-dir button. ("...")
     * Opens the pickFolder dialog.
     * checks if the returned folder is an ftp folder.
     * sets the textbox and the data to the new selected dir.
     */
    public void chooseDirectory()
    {
        SystemDialog sd = SystemDialog.createOfficeFolderDialog(xMSF);
        String newUrl = sd.callFolderDialog(resources.resFTPDirectory, "", getFullUrl());
        if (newUrl != null)
        {
            /*  if the user chose a local directory,
             *  sI do not accept it.
             */
            if (newUrl.startsWith("ftp://"))
            {
                setDir(extractDir(newUrl));
            }
            else
            {
                AbstractErrorHandler.showMessage(xMSF, xControl.getPeer(), resources.resIllegalFolder, ErrorHandler.ERROR_PROCESS_FATAL);
            }
        }
    }

    /**
     * practical to have such a method...
     * @param p the publisher obejct that contains the ftp connection info.
     * @return the full ftp url with username password and everything one needs.
     */
    public static final String getFullURL(CGPublish p)
    {
        return "ftp://" + p.cp_Username + ":" + p.password + "@" + host(p.cp_URL);
    }
}
