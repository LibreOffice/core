/*************************************************************************
 *
 *  $RCSfile: AppletViewer.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:54:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package stardiv.app;

import java.awt.Toolkit;

import java.net.InetAddress;

//import stardiv.applet.AppletMessageHandler;
import stardiv.applet.AppletExecutionContext;
import stardiv.applet.DocumentProxy;

//import stardiv.js.ip.RootTaskManager;
//import stardiv.js.ip.BaseObj;
//import stardiv.js.ide.Ide;
//import stardiv.js.ne.RunTime;
//import stardiv.js.base.IdentifierPool;
//import stardiv.js.base.Identifier;
//import stardiv.memory.AtomUnion;
//import stardiv.js.ip.Ctor;
import stardiv.controller.SjSettings;

import java.util.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.net.URL;
import java.net.MalformedURLException;


/**
 * A frame to show the applet tag in.
 */
class TextFrame extends Frame {
    /**
     * Create the tag frame.
     */
    TextFrame(int x, int y, String title, String text) {
        setTitle(title);
        TextArea txt = new TextArea(20, 60);
        txt.setText(text);
        txt.setEditable(false);

        add("Center", txt);

        Panel p = new Panel();
        add("South", p);
        Button b = new Button(amh.getMessage("button.dismiss", "Dismiss"));
        p.add(b);

        class ActionEventListener implements ActionListener {
            public void actionPerformed(ActionEvent evt) {
                dispose();
            }
        }
        b.addActionListener(new ActionEventListener());

        pack();
        setLocation(x, y);
        setVisible(true);

        WindowListener windowEventListener = new WindowAdapter() {

            public void windowClosing(WindowEvent evt) {
                dispose();
            }
        };

        addWindowListener(windowEventListener);
    }
    private static AppletMessageHandler amh = new AppletMessageHandler("textframe");

}

/**
 * The toplevel applet viewer.
 */
public class AppletViewer extends Frame implements Observer {
    com.sun.star.lib.sandbox.ResourceViewer resourceViewer;

    /**
     * Some constants...
     */
    private static String defaultSaveFile = "Applet.ser";

    /**
     * Look here for the properties file
     */
    public static File theUserPropertiesFile;
    public static File theAppletViewerPropsFile;

    //private Ide aIde;
    //private RootTaskManager aRTM;
    //private BaseObj aRootObj;

    private AppletExecutionContext appletExecutionContext = null;
    Hashtable atts = null;

    static DocumentProxy documentViewer = null;

    /**
     * The status line.
     */
    Label label;

    /**
     * output status messages to this stream
     */

    PrintStream statusMsgStream;

    static Vector contexts = new Vector();

    private final class UserActionListener implements ActionListener {
        public void actionPerformed(ActionEvent evt) {
            processUserAction(evt);
        }
    }

    static {
        String sep = File.separator;

        File userHome = new File(System.getProperty("user.home"));

        File AVHome = new File(userHome, ".hotjava");
        // ensure the props folder can be made
        AVHome.mkdirs();

        theUserPropertiesFile = new File(AVHome, "properties");
        File JH = new File(System.getProperty("java.home"));
        theAppletViewerPropsFile = new File(JH, "lib" + sep + "appletviewer.properties");
    };

    /**
     * Create the applet viewer
     */
    public AppletViewer(int x, int y, URL doc, Hashtable atts, PrintStream statusMsgStream) {
//          resourceViewer = new stardiv.util.ResourceViewer();
//          resourceViewer.show();
        System.err.println("#*#*#*:" + sun.awt.ScreenUpdater.updater);
        this.statusMsgStream = statusMsgStream;
        this.atts = atts;

        setTitle(amh.getMessage("tool.title", atts.get("code")));

        MenuBar mb = new MenuBar();

        Menu m = new Menu(amh.getMessage("menu.applet"));

        addMenuItem(m, "menuitem.restart");
        addMenuItem(m, "menuitem.reload");
        addMenuItem(m, "menuitem.stop");
        addMenuItem(m, "menuitem.save");
        addMenuItem(m, "menuitem.start");
        addMenuItem(m, "menuitem.clone");
        m.add(new MenuItem("-"));
        addMenuItem(m, "menuitem.tag");
        addMenuItem(m, "menuitem.info");
        addMenuItem(m, "menuitem.edit").setEnabled( false );
        addMenuItem(m, "menuitem.encoding");
        m.add(new MenuItem("-"));
        addMenuItem(m, "menuitem.print");
        m.add(new MenuItem("-"));
        addMenuItem(m, "menuitem.props");
        m.add(new MenuItem("-"));
        addMenuItem(m, "menuitem.close");
        //  if (factory.isStandalone()) {
        addMenuItem(m, "menuitem.quit");
        //  }

        mb.add(m);

        setMenuBar(mb);

        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                appletExecutionContext.shutdown();
            }

            public void windowIconified(WindowEvent evt) {
                appletExecutionContext.sendLoad();
            }

            public void windowDeiconified(WindowEvent evt) {
                appletExecutionContext.sendStart();
            }
        });

        add("South", label = new Label(amh.getMessage("label.hello")));

        appletExecutionContext = new AppletExecutionContext(doc, atts, this, 0);
        appletExecutionContext.init();

        appletExecutionContext.addObserver(this);
        contexts.addElement(appletExecutionContext);

        pack();
        setVisible(true);

//          appletExecutionContext.send();
        appletExecutionContext.startUp();

/*
        if( atts.get( "mayscript" ) != null ) {
            aIde = new Ide();
            aRTM = aIde.getActRootTaskManager();
            aRootObj = new BaseObj( aRTM );
            //Ctor aCtor = new AppletCtor( aRTM, "Window" );
            //aRootObj.initProperties( aCtor, aCtor.getStaticPropCount(),   aCtor.getBasePropCount() );
            //aRootObj.setCtor( aCtor );
            aRTM.setRootObj( aRootObj );
            RunTime aRT = new RunTime( aRootObj, aRTM );
            aIde.setRootObj( aRootObj );

            AtomUnion aAU = new AtomUnion();
            BaseObj aDocument = new BaseObj( aRTM );
            aAU.setObject( aDocument );
            Identifier aId = IdentifierPool.aGlobalPool.addIdentifier( "RootObject" );
            aRootObj.newProperty( aId, aAU );
            IdentifierPool.aGlobalPool.releaseIdentifier( aId );

            String pName = (String)atts.get( "name" );
            if( pName != null ) {
                BaseObj aApplet = new BaseObj( aRTM );
                aAU.setObject( aApplet );
                aId = IdentifierPool.aGlobalPool.addIdentifier( pName );
                aDocument.newProperty( aId, aAU );
                IdentifierPool.aGlobalPool.releaseIdentifier( aId );
            }
        }
        */
    }

    public MenuItem addMenuItem(Menu m, String s) {
        MenuItem mItem = new MenuItem(amh.getMessage(s));
        mItem.addActionListener(new UserActionListener());
        return m.add(mItem);
    }

    /**
     * Ignore.
     */
    public void showDocument(URL url) {
    }

    /**
     * Ignore.
     */
    public void showDocument(URL url, String target) {
    }

    /**
     * Show status.
     */
    public void showStatus(String status) {
        label.setText(status);
    }

    public void update(Observable observable, Object status) {
        showStatus((String)status);
    }

    public Object getJavaScriptJSObjectWindow() {
        //if( aRootObj != null )
        //  return aRootObj.getJSObject();
        return null;
    }


    /**
     * System parameters.
     */
    static Hashtable systemParam = new Hashtable();

    static {
        systemParam.put("codebase", "codebase");
        systemParam.put("code", "code");
        systemParam.put("alt", "alt");
        systemParam.put("width", "width");
        systemParam.put("height", "height");
        systemParam.put("align", "align");
        systemParam.put("vspace", "vspace");
        systemParam.put("hspace", "hspace");
    }

    /**
     * Print the HTML tag.
     */
    public static void printTag(PrintStream out, Hashtable atts) {
        out.print("<applet");

        String v = (String)atts.get("codebase");
        if (v != null) {
            out.print(" codebase=\"" + v + "\"");
        }

        v = (String)atts.get("code");
        if (v == null) {
            v = "applet.class";
        }
        out.print(" code=\"" + v + "\"");
        v = (String)atts.get("width");
        if (v == null) {
            v = "150";
        }
        out.print(" width=" + v);

        v = (String)atts.get("height");
        if (v == null) {
            v = "100";
        }
        out.print(" height=" + v);

        v = (String)atts.get("name");
        if (v != null) {
            out.print(" name=\"" + v + "\"");
        }
        out.println(">");

        // A very slow sorting algorithm
        int len = atts.size();
        String params[] = new String[len];
        len = 0;
        for (Enumeration e = atts.keys() ; e.hasMoreElements() ;) {
            String param = (String)e.nextElement();
            int i = 0;
            for (; i < len ; i++) {
                if (params[i].compareTo(param) >= 0) {
                    break;
                }
            }
            System.arraycopy(params, i, params, i + 1, len - i);
            params[i] = param;
            len++;
        }

        for (int i = 0 ; i < len ; i++) {
            String param = params[i];
            if (systemParam.get(param) == null) {
                out.println("<param name=" + param +
                            " value=\"" + atts.get(param) + "\">");
            }
        }
        out.println("</applet>");
    }

    /**
     * Make sure the atrributes are uptodate.
     */
    public void updateAtts() {
        Dimension d = getSize();
        Insets in = getInsets();
        atts.put("width", new Integer(d.width - (in.left + in.right)).toString());
        atts.put("height", new Integer(d.height - (in.top + in.bottom)).toString());
    }

    /**
     * Save the applet to a well known file (for now) as a serialized object
     */
    void appletSave() {
        // REMIND -- should check that the applet has really stopped
        FileDialog fd = new FileDialog(this, "Serialize Applet into File", FileDialog.SAVE);
        // needed for a bug under Solaris...
        fd.setDirectory(System.getProperty("user.dir"));
        fd.setFile(defaultSaveFile);
        fd.show();
        String fname = fd.getFile();
        if (fname == null) {
            return;     // cancelled
        }
        String dname = fd.getDirectory();
        File file = new File(dname, fname);

        try {
            OutputStream s = new FileOutputStream(file);
            ObjectOutputStream os = new ObjectOutputStream(s);
            showStatus(amh.getMessage("appletsave.err1",
                                      appletExecutionContext.getApplet().toString(), file.toString()));
            os.writeObject(appletExecutionContext.getApplet());
        } catch (IOException ex) {
            System.err.println(amh.getMessage("appletsave.err2", ex));
        }
    }

    /**
     * Clone the viewer and the applet.
     */
    void appletClone() {
        Point p = getLocation();
        updateAtts();
        //  factory.createAppletViewer(p.x + 30, p.y + 10,
        //             pHelper.panel.documentURL, (Hashtable)pHelper.panel.atts.clone());
    }

    /**
     * Show the applet tag.
     */
    void appletTag() {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        updateAtts();
        printTag(new PrintStream(out), atts);
        showStatus(amh.getMessage("applettag"));

        Point p = getLocation();
        new TextFrame(p.x + 50, p.y + 20, amh.getMessage("applettag.textframe"), out.toString());
    }

    /**
     * Show the applet info.
     */
    void appletInfo() {
        String str = appletExecutionContext.getApplet().getAppletInfo();
        if (str == null) {
            str = amh.getMessage("appletinfo.applet");
        }
        str += "\n\n";

        String atts[][] = appletExecutionContext.getApplet().getParameterInfo();
        if (atts != null) {
            for (int i = 0 ; i < atts.length ; i++) {
                str += atts[i][0] + " -- " + atts[i][1] + " -- " + atts[i][2] + "\n";
            }
        } else {
            str += amh.getMessage("appletinfo.param");
        }

        Point p = getLocation();
        new TextFrame(p.x + 50, p.y + 20, amh.getMessage("appletinfo.textframe"), str);

    }

    /**
     * Show character encoding type
     */
    void appletCharacterEncoding() {
        showStatus(amh.getMessage("appletencoding", encoding));
    }

    /**
     * Edit the applet.
     */
    void appletEdit() {
    }

    /**
     * Print the applet.
     */
    void appletPrint() {
        PrintJob pj = Toolkit.getDefaultToolkit().
            getPrintJob(this, amh.getMessage("appletprint.printjob"), (Properties)null);


        if (pj != null) {
            Dimension pageDim = pj.getPageDimension();
            int pageRes = pj.getPageResolution();
            boolean lastFirst = pj.lastPageFirst();

            Graphics g = pj.getGraphics();
            if (g != null) {
                appletExecutionContext.getApplet().printComponents(g);
                g.dispose();
            } else {
                statusMsgStream.println(amh.getMessage("appletprint.fail"));
            }
            statusMsgStream.println(amh.getMessage("appletprint.finish"));
            pj.end();

        } else {
            statusMsgStream.println(amh.getMessage("appletprint.cancel"));
        }
    }

    /**
     * Properties.
     */
    AppletProps props;
    public synchronized void networkProperties() {
        if (props == null) {
            props = new AppletProps(appletExecutionContext);
        }
        props.addNotify();
        props.setVisible(true);
    }

    /**
     * Close this viewer.
     * Stop, Destroy, Dispose and Quit an AppletView, then
     * reclaim resources and exit the program if this is
     * the last applet.
     */
    public void appletClose() {
        appletExecutionContext.shutdown();
        contexts.removeElement(this);

        if (contexts.size() == 0) {
            appletSystemExit();
        }
    }


//      public static void writeClasses() {
//          try {
//              java.io.FileOutputStream file = new FileOutputStream("classes.txt");
//              java.io.PrintStream printStream = new java.io.PrintStream(file);

//              printStream.println("- .* .*");
//              Enumeration elements = stardiv.util.HardClassContext.classList.elements();
//              while(elements.hasMoreElements()) {
//                  String string = (String)elements.nextElement();

//                  String packageName = "";
//                  String className = string;

//                  int lastIndex = string.lastIndexOf('.');
//                  if(lastIndex > -1) {
//                      packageName = string.substring(0, lastIndex);
//                      className = string.substring(lastIndex + 1);
//                  }

//                  printStream.print("+ ");
//                  int index;
//                  while((index = packageName.indexOf('.')) > -1) {
//                      printStream.print(packageName.substring(0, index) + "\\\\");
//                      packageName = packageName.substring(index + 1);
//                  }
//                  printStream.print(packageName + "     ");

//                  while((index = className.indexOf('$')) > -1) {
//                      printStream.print(className.substring(0, index) + "\\$");
//                      className = className.substring(index + 1);
//                  }
//                  printStream.println(className + "\\.class");
//              }
//              file.close();
//          }
//          catch(java.io.IOException eio) {
//              System.err.println("IOException:" + eio);
//          }
//      }

    /**
     * Exit the program.
     * Exit from the program (if not stand alone) - do no clean-up
     */
    private void appletSystemExit() {
        //  if (factory.isStandalone())
        System.exit(0);
    }

    /**
     * Quit all viewers.
     * Shutdown all viewers properly then
     * exit from the program (if not stand alone)
     */
    protected void appletQuit() {
        appletExecutionContext.shutdown();
        appletSystemExit();
    }

    /**
     * Handle events.
     */
    public void processUserAction(ActionEvent evt) {

        String label = ((MenuItem)evt.getSource()).getLabel();

        if (amh.getMessage("menuitem.restart").equals(label)) {
            appletExecutionContext.restart();
            return;
        }

        if (amh.getMessage("menuitem.reload").equals(label)) {
            appletExecutionContext.reload();
            return;
        }

        if (amh.getMessage("menuitem.clone").equals(label)) {
            appletClone();
            return;
        }

        if (amh.getMessage("menuitem.stop").equals(label)) {
            appletExecutionContext.sendStop();
            return;
        }

        if (amh.getMessage("menuitem.save").equals(label)) {
            appletSave();
            return;
        }

        if (amh.getMessage("menuitem.start").equals(label)) {
            appletExecutionContext.sendStart();
            return;
        }

        if (amh.getMessage("menuitem.tag").equals(label)) {
            appletTag();
            return;
        }

        if (amh.getMessage("menuitem.info").equals(label)) {
            appletInfo();
            return;
        }

        if (amh.getMessage("menuitem.encoding").equals(label)) {
            appletCharacterEncoding();
            return;
        }

        if (amh.getMessage("menuitem.edit").equals(label)) {
            appletEdit();
            return;
        }

        if (amh.getMessage("menuitem.print").equals(label)) {
            appletPrint();
            return;
        }

        if (amh.getMessage("menuitem.props").equals(label)) {
            networkProperties();
            return;
        }

        if (amh.getMessage("menuitem.close").equals(label)) {
            appletClose();
            return;
        }

        if (/*factory.isStandalone() && */amh.getMessage("menuitem.quit").equals(label)) {
            appletQuit();
            return;
        }
        //statusMsgStream.println("evt = " + evt);
    }

    /**
     * Prepare the enviroment for executing applets.
     */
    public static void init() {
        Properties props = new Properties();
        props.put( "http.proxyHost", "wwwproxy" );
        props.put( "http.proxyPort", "3128" );
        props.put( "ftpProxySet", "true" );
        props.put( "ftpProxyHost", "wwwproxy" );
        props.put( "ftpProxyPort", "3128" );
        props.put( "ftpProxyPort", "3128" );
        props.put( "stardiv.debug.trace", "window" );
        props.put( "stardiv.debug.warning", "window" );
        props.put( "stardiv.debug.error", "window" );
          props.put( "stardiv.security.defaultSecurityManager", "true" );

        // Try loading the appletviewer properties file to get messages, etc.
//          try {
//              FileInputStream in = new FileInputStream(theAppletViewerPropsFile);
//              props.load(new BufferedInputStream(in));
//              in.close();
//          } catch (Exception e) {
//              System.out.println(amh.getMessage("init.err"));
//          }

        // Try loading the saved user properties file to override some
        // of the above defaults.
        try {
            FileInputStream in = new FileInputStream(theUserPropertiesFile);
            props.load(new BufferedInputStream(in));
            in.close();
        } catch (Exception e) {
            /* is it really necessary to say this?
               This is always the case the first time we run..
               System.out.println("[no properties loaded, using defaults]"); */
        }

        // Install a property list.

          SjSettings.changeProperties(props);
    }

    /**
     * The current character.
     */
    static int c;

    /**
     * Scan spaces.
     */
    public static void skipSpace(Reader in) throws IOException {
        while ((c >= 0) && ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))) {
            c = in.read();
        }
    }

    /**
     * Scan identifier
     */
    public static String scanIdentifier(Reader in) throws IOException {
        StringBuffer buf = new StringBuffer();
        while (true) {
            if (((c >= 'a') && (c <= 'z')) ||
                ((c >= 'A') && (c <= 'Z')) ||
                ((c >= '0') && (c <= '9')) || (c == '_')) {
                buf.append((char)c);
                c = in.read();
            } else {
                return buf.toString();
            }
        }
    }

    /**
     * Scan tag
     */
    public static Hashtable scanTag(Reader in) throws IOException {
        Hashtable atts = new Hashtable();
        skipSpace(in);
        while (c >= 0 && c != '>') {
            String att = scanIdentifier(in);
            String val = "";
            skipSpace(in);
            if (c == '=') {
                int quote = -1;
                c = in.read();
                skipSpace(in);
                if ((c == '\'') || (c == '\"')) {
                    quote = c;
                    c = in.read();
                }
                StringBuffer buf = new StringBuffer();
                while ((c > 0) &&
                       (((quote < 0) && (c != ' ') && (c != '\t') &&
                         (c != '\n') && (c != '\r') && (c != '>'))
                        || ((quote >= 0) && (c != quote)))) {
                    buf.append((char)c);
                    c = in.read();
                }
                if (c == quote) {
                    c = in.read();
                }
                skipSpace(in);
                val = buf.toString();
            }
            //statusMsgStream.println("PUT " + att + " = '" + val + "'");
            atts.put(att.toLowerCase(), val);
            skipSpace(in);
        }
        return atts;
    }

    static int x = 100;
    static int y = 50;

    static String encoding = null;

    static private Reader makeReader(InputStream is) {
        if (encoding != null) {
            try {
                return new BufferedReader(new InputStreamReader(is, encoding));
            } catch (IOException x) { }
        }
        InputStreamReader r = new InputStreamReader(is);
        encoding = r.getEncoding();
        return new BufferedReader(r);
    }

    /**
     * Scan an html file for <applet> tags
     */
    public static void parse(URL url) throws IOException {
        parse(url, System.out);
    }

    public static void parse(URL url, PrintStream statusMsgStream) throws IOException {

        // warning messages
        String requiresNameWarning = amh.getMessage("parse.warning.requiresname");
        String paramOutsideWarning = amh.getMessage("parse.warning.paramoutside");
        String requiresCodeWarning = amh.getMessage("parse.warning.requirescode");
        String requiresHeightWarning = amh.getMessage("parse.warning.requiresheight");
        String requiresWidthWarning = amh.getMessage("parse.warning.requireswidth");
        String appNotLongerSupportedWarning = amh.getMessage("parse.warning.appnotLongersupported");

        java.net.URLConnection conn = url.openConnection();
        Reader in = makeReader(conn.getInputStream());
        /* The original URL may have been redirected - this
         * sets it to whatever URL/codebase we ended up getting
         */
        url = conn.getURL();

        Hashtable atts = null;
        while(true) {
            c = in.read();
            if (c == -1)
                break;

            if (c == '<') {
                c = in.read();
                if (c == '/') {
                    c = in.read();
                    String nm = scanIdentifier(in);
                    if (nm.equalsIgnoreCase("applet")) {
                        if (atts != null) {
                            new AppletViewer(x, y, url, atts, System.out);
                            x += 50;
                            y += 20;
                            // make sure we don't go too far!
                            Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
                            if (x > d.width - 30)
                                x = 100;
                            if (y > d.height - 30)
                                y = 50;
                        }
                        atts = null;
                    }
                }
                else {
                    String nm = scanIdentifier(in);
                    if (nm.equalsIgnoreCase("param")) {
                        Hashtable t = scanTag(in);
                        String att = (String)t.get("name");
                        if (att == null) {
                            statusMsgStream.println(requiresNameWarning);
                        } else {
                            String val = (String)t.get("value");
                            if (val == null) {
                                statusMsgStream.println(requiresNameWarning);
                            } else if (atts != null) {
                                atts.put(att.toLowerCase(), val);
                            } else {
                                statusMsgStream.println(paramOutsideWarning);
                            }
                        }
                    }
                    else if (nm.equalsIgnoreCase("applet")) {
                        atts = scanTag(in);
                        if (atts.get("code") == null && atts.get("object") == null) {
                            statusMsgStream.println(requiresCodeWarning);
                            atts = null;
                        } else if (atts.get("width") == null) {
                            statusMsgStream.println(requiresWidthWarning);
                            atts = null;
                        } else if (atts.get("height") == null) {
                            statusMsgStream.println(requiresHeightWarning);
                            atts = null;
                        }
                    }
                    else if (nm.equalsIgnoreCase("app")) {
                        statusMsgStream.println(appNotLongerSupportedWarning);
                        Hashtable atts2 = scanTag(in);
                        nm = (String)atts2.get("class");
                        if (nm != null) {
                            atts2.remove("class");
                            atts2.put("code", nm + ".class");
                        }
                        nm = (String)atts2.get("src");
                        if (nm != null) {
                            atts2.remove("src");
                            atts2.put("codebase", nm);
                        }
                        if (atts2.get("width") == null) {
                            atts2.put("width", "100");
                        }
                        if (atts2.get("height") == null) {
                            atts2.put("height", "100");
                        }
                        printTag(statusMsgStream, atts2);
                        statusMsgStream.println();
                    }
                }
            }
        }
        in.close();
    }

    /**
     * Print usage
     */
    static void usage() {
        System.out.println(amh.getMessage("usage"));
    }

    static boolean didInitialize = false;

    /**
     * mainInit can be called by direct clients
     */
    public static void mainInit() {
        if (! didInitialize) {
            didInitialize = true;

            init();

        }
    }

    /**
     * Main
     */
    public static void main(String argv[]) {
        mainInit();

        // Parse arguments
        if (argv.length == 0) {
            System.out.println(amh.getMessage("main.err.inputfile"));
            usage();
            return;
        }

        // Parse the documents
        for (int i = 0 ; i < argv.length ; i++) {
            try {
                URL url = null;

                if (argv[i].equals("-encoding")) {
                    if(i + 1 < argv.length) {
                        i++;
                        encoding = argv[i];
                        continue;
                    } else {
                        usage();
                        System.exit(1);
                    }
                }
                else
                    if (argv[i].indexOf(':') <= 1) {
                        String userDir = System.getProperty("user.dir");
                        String prot;
                        // prepend native separator to path iff not present
                        if (userDir.charAt(0) == '/' ||
                            userDir.charAt(0) == java.io.File.separatorChar) {
                            prot = "file:";
                        } else {
                            prot = "file:/";
                        }
                        url = new URL(prot + userDir.replace(File.separatorChar, '/')
                                      + "/");
                        url = new URL(url, argv[i]);
                    } else {
                        url = new URL(argv[i]);
                    }

                parse(url);
                documentViewer = DocumentProxy.getDocumentProxy(url, Toolkit.getDefaultToolkit());
            } catch (MalformedURLException e) {
                System.out.println(amh.getMessage("main.err.badurl", argv[i], e.getMessage()));
                System.exit(1);
            } catch (IOException e) {
                System.out.println(amh.getMessage("main.err.io", e.getMessage()));
                if (argv[i].indexOf(':') < 0) {
                    System.out.println(amh.getMessage("main.err.readablefile", argv[i]));
                } else {
                    System.out.println(amh.getMessage("main.err.correcturl", argv[i]));
                }
                System.exit(1);
            }
        }
        /*
          if (documentViewer.countApplets() == 0) {
          System.out.println(amh.getMessage("main.warning"));
          usage();
          System.exit(1);
          }*/
    }
    private static AppletMessageHandler amh = new AppletMessageHandler("appletviewer");
}
