/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MsgAppletViewer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:17:48 $
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
package stardiv.applet.resources;

import java.util.ListResourceBundle;

public class MsgAppletViewer extends ListResourceBundle {

    public Object[][] getContents() {
    return contents;
    }

    static final Object[][] contents = {
    {"textframe.button.dismiss", "Dismiss"},
    {"appletviewer.tool.title", "Applet Viewer: {0}"},
    {"appletviewer.menu.applet", "Applet"},
    {"appletviewer.menuitem.restart", "Restart"},
    {"appletviewer.menuitem.reload", "Reload"},
    {"appletviewer.menuitem.stop", "Stop"},
    {"appletviewer.menuitem.save", "Save..."},
    {"appletviewer.menuitem.start", "Start"},
    {"appletviewer.menuitem.clone", "Clone..."},
    {"appletviewer.menuitem.tag", "Tag..."},
    {"appletviewer.menuitem.info", "Info..."},
    {"appletviewer.menuitem.edit", "Edit"},
    {"appletviewer.menuitem.encoding", "Character Encoding"},
    {"appletviewer.menuitem.print", "Print..."},
    {"appletviewer.menuitem.props", "Properties..."},
    {"appletviewer.menuitem.close", "Close"},
    {"appletviewer.menuitem.quit", "Quit"},
    {"appletviewer.label.hello", "Hello..."},
    {"appletviewer.status.start", "starting applet..."},
    {"appletviewer.appletsave.err1", "serializing an {0} to {1}"},
    {"appletviewer.appletsave.err2", "in appletSave: {0}"},
    {"appletviewer.applettag", "Tag shown"},
    {"appletviewer.applettag.textframe", "Applet HTML Tag"},
    {"appletviewer.appletinfo.applet", "-- no applet info --"},
    {"appletviewer.appletinfo.param", "-- no parameter info --"},
    {"appletviewer.appletinfo.textframe", "Applet Info"},
    {"appletviewer.appletprint.printjob", "Print Applet"},
    {"appletviewer.appletprint.fail", "Printing failed."},
    {"appletviewer.appletprint.finish", "Finished printing."},
    {"appletviewer.appletprint.cancel", "Printing cancelled."},
    {"appletviewer.appletencoding", "Character Encoding: {0}"},
    {"appletviewer.init.err", "[no appletviewer.properties file found!]"},
    {"appletviewer.parse.warning.requiresname", "Warning: <param name=... value=...> tag requires name attribute."},
    {"appletviewer.parse.warning.paramoutside", "Warning: <param> tag outside <applet> ... </applet>."},
    {"appletviewer.parse.warning.requirescode", "Warning: <applet> tag requires code attribute."},
    {"appletviewer.parse.warning.requiresheight", "Warning: <applet> tag requires height attribute."},
    {"appletviewer.parse.warning.requireswidth", "Warning: <applet> tag requires width attribute."},
    {"appletviewer.parse.warning.appnotLongersupported", "Warning: <app> tag no longer supported, use <applet> instead:"},
    {"appletviewer.usage", "usage: appletviewer [-debug] [-J<javaflag>] [-encoding <character encoding type> ] url|file ..."},
    {"appletviewer.main.err.inputfile", "No input files specified."},
    {"appletviewer.main.err.badurl", "Bad URL: {0} ( {1} )"},
    {"appletviewer.main.err.io", "I/O exception while reading: {0}"},
    {"appletviewer.main.err.readablefile", "Make sure that {0} is a file and is readable."},
    {"appletviewer.main.err.correcturl", "Is {0} the correct URL?"},
    {"appletviewer.main.warning", "Warning: No Applets were started. Make sure the input contains an <applet> tag."},
    {"appletpanel.runloader.err", "Either object or code parameter!"},
    {"appletpanel.runloader.exception", "exception while deserializing {0}"},
    {"appletpanel.destroyed", "Applet destroyed."},
    {"appletpanel.loaded", "Applet loaded."},
    {"appletpanel.started", "Applet started."},
    {"appletpanel.inited", "Applet initialized."},
    {"appletpanel.stopped", "Applet stopped."},
    {"appletpanel.disposed", "Applet disposed."},
    {"appletpanel.nocode", "APPLET tag missing CODE parameter."},
    {"appletpanel.notfound", "load: class {0} not found."},
    {"appletpanel.nocreate", "load: {0} can''t be instantiated."},
    {"appletpanel.noconstruct", "load: {0} is not public or has no public constructor."},
    {"appletpanel.death", "killed"},
    {"appletpanel.exception", "exception: {0}."},
    {"appletpanel.exception2", "exception: {0}: {1}."},
    {"appletpanel.error", "error: {0}."},
    {"appletpanel.error2", "error: {0}: {1}."},
    {"appletpanel.notloaded", "Init: applet not loaded."},
    {"appletpanel.notinited", "Start: applet not initialized."},
    {"appletpanel.notstarted", "Stop: applet not started."},
    {"appletpanel.notstopped", "Destroy: applet not stopped."},
    {"appletpanel.notdestroyed", "Dispose: applet not destroyed."},
    {"appletpanel.notdisposed", "Load: applet not disposed."},
    {"appletpanel.bail", "Interrupted: bailing out."},
    {"appletpanel.filenotfound", "File not found when looking for: {0}"},
    {"appletpanel.fileformat", "File format exception when loading: {0}"},
    {"appletpanel.fileioexception", "I/O exception when loading: {0}"},
    {"appletpanel.fileexception", "{0} exception when loading: {1}"},
    {"appletpanel.filedeath", "{0} killed when loading: {1}"},
    {"appletpanel.fileerror", "{0} error when loading: {1}"},
    {"appletillegalargumentexception.objectinputstream", "AppletObjectInputStream requires non-null loader"},
    {"appletprops.title", "AppletViewer Properties"},
    {"appletprops.label.http.server", "Http proxy server:"},
    {"appletprops.label.http.proxy", "Http proxy port:"},
    {"appletprops.label.network", "Network access:"},
    {"appletprops.choice.network.item.none", "None"},
    {"appletprops.choice.network.item.applethost", "Applet Host"},
    {"appletprops.choice.network.item.unrestricted", "Unrestricted"},
    {"appletprops.label.class", "Class access:"},
    {"appletprops.choice.class.item.restricted", "Restricted"},
    {"appletprops.choice.class.item.unrestricted", "Unrestricted"},
    {"appletprops.label.unsignedapplet", "Allow unsigned applets:"},
    {"appletprops.choice.unsignedapplet.no", "No"},
    {"appletprops.choice.unsignedapplet.yes", "Yes"},
    {"appletprops.button.apply", "Apply"},
    {"appletprops.button.cancel", "Cancel"},
    {"appletprops.button.reset", "Reset"},
    {"appletprops.apply.exception", "Failed to save properties: {0}"},
    };
}
