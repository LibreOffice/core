/*************************************************************************
 *
 *  $RCSfile: _XURLTransformer.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-10-06 13:31:35 $
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

package ifc.util;

import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.util.XURLTransformer</code>
* interface methods :
* <ul>
*  <li><code> assemble() </code></li>
*  <li><code> parseStrict() </code></li>
*  <li><code> parseSmart() </code></li>
*  <li><code> getPresentation() </code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XURLTransformer
*/
public class _XURLTransformer extends MultiMethodTest {

    public XURLTransformer oObj = null;

    URL url;

    final static String user = "user";
    final static String password = "password";
    final static String server = "server";
    final static String port = "8080";
    final static String path = "/pub/path";
    final static String name = "file.txt";
    final static String arguments = "a=b";
    final static String mark = "mark";

    final static String expectedCompleteHTTP = "http://"
                + server + ":" + port + path
                + "/" + name + "?" + arguments + "#" + mark;
    final static String expectedCompleteFTP = "ftp://"
                + user + ":" + password + "@" + server + ":" + port + path
                + "/" + name;

    /**
     * First the complete URL (all URL fields are filled) is
     * passed and assembled. Then incomplete URL (only
     * <code>Server</code> field is set) is passed. <p>
     * Has <b> OK </b> status if in the first case <code>true</code>
     * retruned and <code>Complete</code> field is set and in the
     * second case <code>false</code> is returned. <p>
     */
    public void _assemble(){
        URL[] url = new URL[1];
        url[0] = new URL();

        url[0].Protocol = "http://";
        url[0].Server = server;
        url[0].Port = new Integer(port).shortValue();
        url[0].Path = path;
        url[0].Name = name;
        url[0].Arguments = arguments;
        url[0].Mark = mark;
        url[0].Main = "http://" + server + ":" +
            port + path + "/" + name;

        boolean res = true;

        log.print("assemble http-URL: ");
        boolean complete = oObj.assemble(url);
        log.println(complete);
        res &= complete;

        if (!expectedCompleteHTTP.equals(url[0].Complete)) {
            log.println("assemble works wrong");
            log.println("complete field : " + url[0].Complete);
            log.println("expected : " + expectedCompleteHTTP);
            res = false;
        }

        url[0] = new URL();
        url[0].Protocol = "ftp://";
        url[0].User = user;
        url[0].Password = password;
        url[0].Server = server;
        url[0].Port = new Integer(port).shortValue();
        url[0].Path = path;
        url[0].Name = name;
        url[0].Main = "ftp://" + user + ":" + password + "@" + server + ":" +
            port + path + "/" + name;

        log.print("assemble ftp-URL: ");
        complete = oObj.assemble(url);
        log.println(complete);
        res &= complete;

        if (!expectedCompleteFTP.equals(url[0].Complete)) {
            log.println("assemble works wrong");
            log.println("complete field : " + url[0].Complete);
            log.println("expected : " + expectedCompleteFTP);
            res = false;
        }

        URL[] incompleteUrl = new URL[1];
        incompleteUrl[0] = new URL();
        incompleteUrl[0].Server = server;

        log.print("assemble incomplete URL: ");
        complete = oObj.assemble(incompleteUrl);
        log.println(complete);
        res &= !complete;

        // should be incomplete
        tRes.tested("assemble()", res);
    }

    /**
     * First the complete URL (<code>Complete</code> field is set
     * to proper URL) is passed and parsed. Then incomplete URL (only
     * <code>Server</code> field is set) is passed. <p>
     * Has <b> OK </b> status if in the first case <code>true</code>
     * retruned and all URL fields are set to proper values and in the
     * second case <code>false</code> is returned. <p>
     */
    public void _parseStrict() {
        URL[] url = new URL[1];

        url[0] = new URL();
        url[0].Complete = expectedCompleteHTTP;

        boolean res = true;
        log.print("parseStrict(" + expectedCompleteHTTP + "): ");
        boolean complete = oObj.parseStrict(url);
        log.println(complete);
        res &= complete;

        if (!url[0].Protocol.equals("http://")) {
            log.println("parseStrict works wrong");
            log.println("protocol field : " + url[0].Protocol);
            log.println("expected : http://");
            res = false;
        }

        if (!url[0].Server.equals(server)) {
            log.println("parseStrict works wrong");
            log.println("server field : " + url[0].Server);
            log.println("expected : " + server);
            res = false;
        }

        if (url[0].Port != new Integer(port).shortValue()) {
            log.println("parseStrict works wrong");
            log.println("port field : " + url[0].Port);
            log.println("expected : " + port);
            res = false;
        }

        if ((!url[0].Path.equals(path)) && (!url[0].Path.equals(path + "/"))) {
            log.println("parseStrict works wrong");
            log.println("path field : " + url[0].Path);
            log.println("expected : " + path);
            res = false;
        }

        if (!url[0].Name.equals(name)) {
            log.println("parseStrict works wrong");
            log.println("name field : " + url[0].Name);
            log.println("expected : " + name);
            res = false;
        }

        if (!url[0].Arguments.equals(arguments)) {
            log.println("parseStrict works wrong");
            log.println("arguments field : " + url[0].Arguments);
            log.println("expected : " + arguments);
          res = false;
        }

        if (!url[0].Mark.equals(mark)) {
            log.println("parseStrict works wrong");
            log.println("mark field : " + url[0].Mark);
            log.println("expected : " + mark);
            res = false;
        }

        url[0] = new URL();
        url[0].Complete = expectedCompleteFTP;

        log.print("parseStrict(" + expectedCompleteFTP + "): ");
        complete = oObj.parseStrict(url);
        log.println(complete);
        res &= complete;

        if (!url[0].Protocol.equals("ftp://")) {
            log.println("parseStrict works wrong");
            log.println("protocol field : " + url[0].Protocol);
            log.println("expected : ftp://");
            res = false;
        }

        if (!url[0].User.equals(user)) {
            log.println("parseStrict works wrong");
            log.println("user field : " + url[0].User);
            log.println("expected : " + user);
            res = false;
        }

        if (!url[0].Password.equals(password)) {
            log.println("parseStrict works wrong");
            log.println("password field : " + url[0].Password);
            log.println("expected : " + password);
            res = false;
        }

        if (!url[0].Server.equals(server)) {
            log.println("parseStrict works wrong");
            log.println("server field : " + url[0].Server);
            log.println("expected : " + server);
            res = false;
        }

        if (url[0].Port != new Integer(port).shortValue()) {
            log.println("parseStrict works wrong");
            log.println("port field : " + url[0].Port);
            log.println("expected : " + port);
            res = false;
        }

        if ((!url[0].Path.equals(path)) && (!url[0].Path.equals(path + "/"))) {
            log.println("parseStrict works wrong");
            log.println("path field : " + url[0].Path);
            log.println("expected : " + path);
            res = false;
        }

        if (!url[0].Name.equals(name)) {
            log.println("parseStrict works wrong");
            log.println("name field : " + url[0].Name);
            log.println("expected : " + name);
            res = false;
        }

        URL[] incompleteUrl = new URL[1];
        incompleteUrl[0] = new URL();
        incompleteUrl[0].Complete = server;

        log.print("parseStrict(" + server + "): ");
        complete = oObj.parseStrict(incompleteUrl);
        log.println(complete);
        // should be incomplete
        res &= !complete;

        tRes.tested("parseStrict()", res);
    }

    /**
     * Tries to parse WWW server name. <p>
     * Has <b> OK </b> status if the method return <code>true</code>
     * value and <code>Protocol, Server, Port</code> URL fields are
     * set properly.
     */
    public void _parseSmart() {
        URL[] url = new URL[1];

        String httpURL = server + ":" + port + path + "/" + name + "?" +
            arguments + "#" + mark;

        url[0] = new URL();
        url[0].Complete = httpURL;

        boolean res = true;
        log.print("parseSmart('" + httpURL + "', 'http://'): ");
        boolean complete = oObj.parseSmart(url, "http://");
        log.println(complete);
        res &= complete;

        if (!url[0].Protocol.equals("http://")) {
            log.println("parseSmart works wrong");
            log.println("protocol field : " + url[0].Protocol);
            log.println("expected : http://");
            res = false;
        }

        if (!url[0].Server.equals(server)) {
            log.println("parseSmart works wrong");
            log.println("server field : " + url[0].Server);
            log.println("expected : " + server);
            res = false;
        }

        if (url[0].Port != new Integer(port).shortValue()) {
            log.println("parseSmart works wrong");
            log.println("port field : " + url[0].Port);
            log.println("expected : " + port);
            res = false;
        }

        if ((!url[0].Path.equals(path)) && (!url[0].Path.equals(path + "/"))) {
            log.println("parseSmart works wrong");
            log.println("path field : " + url[0].Path);
            log.println("expected : " + path);
            res = false;
        }

        if (!url[0].Name.equals(name)) {
            log.println("parseSmart works wrong");
            log.println("name field : " + url[0].Name);
            log.println("expected : " + name);
            res = false;
        }

        if (!url[0].Arguments.equals(arguments)) {
            log.println("parseSmart works wrong");
            log.println("arguments field : " + url[0].Arguments);
            log.println("expected : " + arguments);
            res = false;
        }

        if (!url[0].Mark.equals(mark)) {
            log.println("parseSmart works wrong");
            log.println("mark field : " + url[0].Mark);
            log.println("expected : " + mark);
            res = false;
        }

        String ftpURL = user + ":" + password + "@" + server + ":" +
            port + path + "/" + name;

        url[0] = new URL();
        url[0].Complete = ftpURL;
        log.print("parseSmart('" + ftpURL + "', 'ftp://'): ");
        complete = oObj.parseSmart(url, "ftp://");
        log.println(complete);
        res &= complete;

        if (!url[0].Protocol.equals("ftp://")) {
            log.println("parseSmart works wrong");
            log.println("protocol field : " + url[0].Protocol);
            log.println("expected : ftp://");
            res = false;
        }

        if (!url[0].User.equals(user)) {
            log.println("parseSmart works wrong");
            log.println("user field : " + url[0].User);
            log.println("expected : " + user);
            res = false;
        }

        if (!url[0].Password.equals(password)) {
            log.println("parseSmart works wrong");
            log.println("password field : " + url[0].Password);
            log.println("expected : " + password);
            res = false;
        }

        if (!url[0].Server.equals(server)) {
            log.println("parseSmart works wrong");
            log.println("server field : " + url[0].Server);
            log.println("expected : " + server);
            res = false;
        }

        if (url[0].Port != new Integer(port).shortValue()) {
            log.println("parseSmart works wrong");
            log.println("port field : " + url[0].Port);
            log.println("expected : " + port);
            res = false;
        }

        if ((!url[0].Path.equals(path)) && (!url[0].Path.equals(path + "/"))) {
            log.println("parseSmart works wrong");
            log.println("path field : " + url[0].Path);
            log.println("expected : " + path);
            res = false;
        }

        if (!url[0].Name.equals(name)) {
            log.println("parseSmart works wrong");
            log.println("name field : " + url[0].Name);
            log.println("expected : " + name);
            res = false;
        }

        tRes.tested("parseSmart()", res);
    }

    /**
     * Gets the presentation of a URL. <p>
     * Has <b> OK </b> status if the method returns the same
     * URL as was passed in parameter.
     */
    public void _getPresentation() {
        URL url = new URL();

        url.Complete = expectedCompleteHTTP;

        log.println("getPresentation('" + expectedCompleteHTTP + "', true): ");
        String presentation = oObj.getPresentation(url, true);
        boolean res = presentation.equals(expectedCompleteHTTP);
        log.println("Resulted presentation: " + presentation);
        log.println("Expected presentation: " + expectedCompleteHTTP);
        log.println("Result: " + res);

        url.Complete = expectedCompleteFTP;
        log.println("getPresentation('" + expectedCompleteFTP + "', false): ");
        // the password must be masqurade with <****>
        String asterix = "";
        for (int n = 0 ; n < password.length(); n++){
            asterix += "*";
        }
        asterix = "<" + asterix.substring(1,asterix.length());
        asterix = asterix.substring(0,asterix.length()-1) + ">";

        presentation = oObj.getPresentation(url, false);
        String expectedPresentation = "ftp://" + user + ":" + asterix + "@" +
            server + ":" + port + path + "/" + name;
        res &= presentation.equals(expectedPresentation);
        log.println("Resulted presentation: " + presentation);
        log.println("Expected presentation: " + expectedPresentation);
        log.println("Result: " + res);

        log.println("getPresentation('" + expectedCompleteFTP + "', true): ");
        presentation = oObj.getPresentation(url, true);
        expectedPresentation = "ftp://" + user + ":" + password + "@" +
            server + ":" + port + path + "/" + name;
        res &= presentation.equals(expectedPresentation);
        log.println("Resulted presentation: " + presentation);
        log.println("Expected presentation: " + expectedPresentation);
        log.println("Result: " + res);

        String incorrectURL = "*bla-bla*";
        url.Complete = incorrectURL;
        log.println("getPresentation('" + incorrectURL + "', false): ");
        presentation = oObj.getPresentation(url, false);
        expectedPresentation = "";
        res &= presentation.equals(expectedPresentation);
        log.println("Resulted presentation: " + presentation);
        log.println("Expected presentation: " + expectedPresentation);
        log.println("Result: " + res);

        tRes.tested("getPresentation()", res);
    }

}  // finish class _XURLTransformer

