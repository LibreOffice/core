/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MsgAppletViewer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:19:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package stardiv.security.resources;

import java.util.ListResourceBundle;

/* Alle Resourcen aus AppletClassLoader, Applet und AppletSecurityException.
*/
public class MsgAppletViewer extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    {"appletclassloader.loadcode.verbose", "Opening stream to: {0} to get {1}"},
    {"appletclassloader.filenotfound", "File not found when looking for: {0}"},
    {"appletclassloader.fileformat", "File format exception when loading: {0}"},
    {"appletclassloader.fileioexception", "I/O exception when loading: {0}"},
    {"appletclassloader.fileexception", "{0} exception when loading: {1}"},
    {"appletclassloader.filedeath", "{0} killed when loading: {1}"},
    {"appletclassloader.fileerror", "{0} error when loading: {1}"},
    {"appletclassloader.findclass.verbose.findclass", "{0} find class {1}"},
    {"appletclassloader.findclass.verbose.openstream", "Opening stream to: {0} to get {1}"},
    {"appletclassloader.getresource.verbose.forname", "AppletClassLoader.getResource for name: {0}"},
    {"appletclassloader.getresource.verbose.found", "Found resource: {0} as a system resource"},
    {"appletclassloader.getresourceasstream.verbose", "Found resource: {0} as a system resource"},
    {"appletioexception.loadclass.throw.interrupted", "class loading interrupted: {0}"},
    {"appletioexception.loadclass.throw.notloaded", "class not loaded: {0}"},
    {"appletsecurityexception.checkcreateclassloader", "Security Exception: classloader"},
    {"appletsecurityexception.checkaccess.thread", "Security Exception: thread"},
    {"appletsecurityexception.checkaccess.threadgroup", "Security Exception: threadgroup: {0}"},
    {"appletsecurityexception.checkexit", "Security Exception: exit: {0}"},
    {"appletsecurityexception.checkexec", "Security Exception: exec: {0}"},
    {"appletsecurityexception.checklink", "Security Exception: link: {0}"},
    {"appletsecurityexception.checkpropsaccess", "Security Exception: properties"},
    {"appletsecurityexception.checkpropsaccess.key", "Security Exception: properties access {0}"},
    {"appletsecurityexception.checkread.exception1", "Security Exception: {0}, {1}"},
    {"appletsecurityexception.checkread.exception2", "Security Exception: file.read: {0}"},
    {"appletsecurityexception.checkread", "Security Exception: file.read: {0} == {1}"},
    {"appletsecurityexception.checkwrite.exception", "Security Exception: {0}, {1}"},
    {"appletsecurityexception.checkwrite", "Security Exception: file.write: {0} == {1}"},
    {"appletsecurityexception.checkread.fd", "Security Exception: fd.read"},
    {"appletsecurityexception.checkwrite.fd", "Security Exception: fd.write"},
    {"appletsecurityexception.checklisten", "Security Exception: socket.listen: {0}"},
    {"appletsecurityexception.checkaccept", "Security Exception: socket.accept: {0}:{1}"},
    {"appletsecurityexception.checkconnect.networknone", "Security Exception: socket.connect: {0}->{1}"},
    {"appletsecurityexception.checkconnect.networkhost1", "Security Exception: Couldn''t connect to {0} with origin from {1}."},
    {"appletsecurityexception.checkconnect.networkhost2", "Security Exception: Couldn''t resolve IP for host {0} or for {1}. "},
    {"appletsecurityexception.checkconnect.networkhost3", "Security Exception: Could not resolve IP for host {0}. See the trustProxy property."},
    {"appletsecurityexception.checkconnect", "Security Exception: connect: {0}->{1}"},
    {"appletsecurityexception.checkpackageaccess", "Security Exception: cannot access package: {0}"},
    {"appletsecurityexception.checkpackagedefinition", "Security Exception: cannot define package: {0}"},
    {"appletsecurityexception.cannotsetfactory", "Security Exception: cannot set factory"},
    {"appletsecurityexception.checkmemberaccess", "Security Exception: check member access"},
    {"appletsecurityexception.checkgetprintjob", "Security Exception: getPrintJob"},
    {"appletsecurityexception.checksystemclipboardaccess", "Security Exception: getSystemClipboard"},
    {"appletsecurityexception.checkawteventqueueaccess", "Security Exception: getEventQueue"},
    {"appletsecurityexception.checksecurityaccess", "Security Exception: security operation: {0}"},
    {"appletsecurityexception.getsecuritycontext.unknown", "unknown class loader type. unable to check for getContext"},
    {"appletsecurityexception.checkread.unknown", "unknown class loader type. unable to check for checking read {0}"},
    {"appletsecurityexception.checkconnect.unknown", "unknown class loader type. unable to check for checking connect"},
    };
}
