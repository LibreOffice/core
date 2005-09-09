/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PeerConfig.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:47:57 $
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

package com.sun.star.wizards.ui;

import java.util.Vector;

import com.sun.star.awt.*;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class PeerConfig implements XWindowListener{
    Vector aPeerTasks = new Vector();
    Vector aControlTasks = new Vector();
    Vector aImageUrlTasks = new Vector();
    UnoDialog oUnoDialog = null;

    public PeerConfig(UnoDialog _oUnoDialog) {
        oUnoDialog = _oUnoDialog;
        oUnoDialog.xWindow.addWindowListener(this);
    }

    class PeerTask{
        XControl xControl;
        String[] propnames;
        Object[] propvalues;

        public PeerTask(XControl _xControl, String[] propNames_, Object[] propValues_){
            propnames = propNames_;
            propvalues = propValues_;
            xControl = _xControl;
        }
    }


    class ControlTask{
        Object oModel;
        String propname;
        Object propvalue;

        public ControlTask(Object _oModel, String _propName, Object _propValue){
            propname = _propName;
            propvalue = _propValue;
            oModel = _oModel;
        }
    }


    class ImageUrlTask{
        Object oModel;
        Object oResource;
        Object oHCResource;

        public ImageUrlTask(Object _oModel, Object _oResource, Object _oHCResource){
            oResource = _oResource;
            oHCResource = _oHCResource;
            oModel = _oModel;
        }
    }


    public void windowResized(WindowEvent arg0) {
    }

    public void windowMoved(WindowEvent arg0) {
    }


    public void windowShown(EventObject arg0) {
    try {
        for (int i = 0; i < this.aPeerTasks.size(); i++){
            PeerTask aPeerTask = (PeerTask) aPeerTasks.elementAt(i);
            XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, aPeerTask.xControl.getPeer());
            for (int n = 0; n < aPeerTask.propnames.length; n++)
                xVclWindowPeer.setProperty(aPeerTask.propnames[n], aPeerTask.propvalues[n]);
        }
        for (int i = 0; i < this.aControlTasks.size(); i++){
            ControlTask aControlTask = (ControlTask) aControlTasks.elementAt(i);
            Helper.setUnoPropertyValue(aControlTask.oModel, aControlTask.propname, aControlTask.propvalue);
        }
        for (int i = 0; i < this.aImageUrlTasks.size(); i++){
            ImageUrlTask aImageUrlTask = (ImageUrlTask) aImageUrlTasks.elementAt(i);
            String sImageUrl = "";
            if (AnyConverter.isInt(aImageUrlTask.oResource))
                sImageUrl = oUnoDialog.getWizardImageUrl(((Integer) aImageUrlTask.oResource).intValue(), ((Integer) aImageUrlTask.oHCResource).intValue());
            else if (AnyConverter.isString(aImageUrlTask.oResource))
                sImageUrl = oUnoDialog.getImageUrl(((String) aImageUrlTask.oResource), ((String) aImageUrlTask.oHCResource));
            if (!sImageUrl.equals(""))
                Helper.setUnoPropertyValue(aImageUrlTask.oModel, "ImageURL", sImageUrl);
        }

    } catch (RuntimeException re) {
        re.printStackTrace(System.out);
        throw re;
    }}


    public void windowHidden(EventObject arg0) {
    }


    public void disposing(EventObject arg0) {
    }

    /**
     *
     * @param oAPIControl an API control that the interface XControl can be derived from
     * @param _scontrolname
     */
    public void setAccessiblityName(Object oAPIControl, String _saccessname) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oAPIControl);
        setPeerProperties(xControl, new String[] { "AccessibilityName" }, new String[]{_saccessname});
    }


    public void setAccessiblityName(XControl _xControl, String _saccessname) {
        setPeerProperties(_xControl, new String[] { "AccessibilityName" }, new String[]{_saccessname});
    }

    /**
     *
     * @param oAPIControl an API control that the interface XControl can be derived from
     * @param propnames
     * @param propvalues
     */
    public void setPeerProperties(Object oAPIControl, String[] _propnames, Object[] _propvalues) {
        XControl xControl = (XControl) UnoRuntime.queryInterface(XControl.class, oAPIControl);
        setPeerProperties(xControl, _propnames, _propvalues);
    }


    public void setPeerProperties(XControl _xControl, String[] propnames, Object[] propvalues) {
        PeerTask  oPeerTask = new PeerTask(_xControl, propnames, propvalues);
        this.aPeerTasks.add(oPeerTask);
    }

    /**
     * assigns an arbitrary property to a control as soon as the peer is created
     * Note: The property 'ImageUrl' should better be assigned with 'setImageurl(...)', to consider the High Contrast Mode
     * @param _ocontrolmodel
     * @param _spropname
     * @param _propvalue
     */
    public void setControlProperty(Object _ocontrolmodel, String _spropname, Object _propvalue){
        ControlTask oControlTask = new ControlTask(_ocontrolmodel, _spropname, _propvalue);
        this.aControlTasks.add(oControlTask);
    }

    /**
     * Assigns an image to the property 'ImageUrl' of a dialog control. The image id must be assigned in a resource file
     * within the wizards project
     * wizards project
     * @param _ocontrolmodel
     * @param _nResId
     * @param _nhcResId
     */
    public void setImageUrl(Object _ocontrolmodel, int _nResId, int _nhcResId){
        ImageUrlTask oImageUrlTask = new ImageUrlTask(_ocontrolmodel, new Integer(_nResId), new Integer(_nhcResId));
        this.aImageUrlTasks.add(oImageUrlTask);
    }

    /**
     * Assigns an image to the property 'ImageUrl' of a dialog control. The image ids that the Resource urls point to
     * may be assigned in a Resource file outside the wizards project
     * @param _ocontrolmodel
     * @param _sResourceUrl
     * @param _sHCResourceUrl
     */
    public void setImageUrl(Object _ocontrolmodel, String _sResourceUrl, String _sHCResourceUrl){
        ImageUrlTask oImageUrlTask = new ImageUrlTask(_ocontrolmodel, _sResourceUrl, _sHCResourceUrl);
        this.aImageUrlTasks.add(oImageUrlTask);
    }


    /**
     * Assigns an image to the property 'ImageUrl' of a dialog control. The image id must be assigned in a resource file
     * within the wizards project
     * wizards project
     * @param _ocontrolmodel
     * @param _nResId
     * @param _nhcResId
     */
    public void setImageUrl(Object _ocontrolmodel, Object _oResource, Object _oHCResource){
        ImageUrlTask oImageUrlTask = new ImageUrlTask(_ocontrolmodel, _oResource, _oHCResource);
        this.aImageUrlTasks.add(oImageUrlTask);
    }



}
