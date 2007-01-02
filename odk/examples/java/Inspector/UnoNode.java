
/*************************************************************************
 *
 *  $RCSfile: UnoNode.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-01-02 15:02:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.FrameSearchFlag;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.reflection.TypeDescriptionSearchDepth;
import com.sun.star.reflection.XServiceTypeDescription;
import com.sun.star.reflection.XTypeDescription;
import com.sun.star.reflection.XTypeDescriptionEnumeration;
import com.sun.star.reflection.XTypeDescriptionEnumerationAccess;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import java.util.List;
import java.util.Vector;
import javax.swing.tree.TreeNode;


public abstract class UnoNode extends HideableMutableTreeNode{

    String sPath = null;
    Object m_oUnoObject;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private XComponentContext m_xComponentContext;
    protected static String SMETHODDESCRIPTION = "Methods";
    protected static String SPROPERTYDESCRIPTION = "Properties";
    protected static String SPROPERTYINFODESCRIPTION = "PropertyInfo";
    protected static String SCONTAINERDESCRIPTION = "Container";
    protected static String SSERVICEDESCRIPTION = "Services";
    protected static String SINTERFACEDESCRIPTION = "Interfaces";
    protected static String SCONTENTDESCRIPTION = "Content";
    protected static String SPROPERTYVALUEDESCRIPTION = "PropertyValues";



    /** Creates a new instance of UnoNode */
    public UnoNode(Object _oUnoObject) {
        super();
        m_xComponentContext = Introspector.getIntrospector().getXComponentContext();
        m_xMultiComponentFactory = m_xComponentContext.getServiceManager();
        m_oUnoObject = _oUnoObject;
    }


    public Object getUnoObject(){
        return m_oUnoObject;
    }


    protected XComponentContext getXComponentContext(){
        return m_xComponentContext;
    }


    protected XMultiComponentFactory getXMultiComponentFactory(){
        return m_xMultiComponentFactory;
    }


    protected static XTypeDescriptionEnumerationAccess getXTypeDescriptionEnumerationAccess(){
       return Introspector.getIntrospector().getXTypeDescriptionEnumerationAccess();
    }


    public String getAnchor(){
        return "";
    }


    public String getClassName(){
        String sClassName = "";
        TreeNode oTreeNode = getParent();
        if (oTreeNode != null){
           if (oTreeNode instanceof UnoNode){
               UnoNode oUnoNode = (UnoNode) oTreeNode;
               sClassName = oUnoNode.getClassName();
           }
        }
        return sClassName;
    }


    public abstract String getName();



    public void openIdlDescription(){
    try{
        String sClassName = getClassName();
        String sAnchor = getAnchor();
        String sFallbackUrl = "http://api.openoffice.org/docs/common/ref/com/sun/star/module-ix";
        String sIdlUrl = "http://api.openoffice.org/docs/common/ref/";
        if (sClassName.equals("")){
            sIdlUrl = sFallbackUrl;
            sAnchor = "";
        }
        else{
            sIdlUrl += sClassName.replace('.', '/');
        }
        sIdlUrl += ".html";
        if (sAnchor != null){
            if (!sAnchor.equals("")){
                sIdlUrl += "#" + sAnchor;
            }
        }
        URL openHyperlink = getDispatchURL(".uno:OpenHyperlink");
        PropertyValue pv = new PropertyValue();
        pv.Name = "URL";
        pv.Value = sIdlUrl;
        getXDispatcher(openHyperlink).dispatch(openHyperlink, new PropertyValue[] {pv});
    } catch(Exception exception) {
        exception.printStackTrace(System.out);
    }}


    public com.sun.star.util.URL getDispatchURL(String _sURL){
        try {
            Object oTransformer = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.util.URLTransformer", getXComponentContext());
            XURLTransformer xTransformer = (XURLTransformer) UnoRuntime.queryInterface(XURLTransformer.class, oTransformer);
            com.sun.star.util.URL[] oURL = new com.sun.star.util.URL[1];
            oURL[0] = new com.sun.star.util.URL();
            oURL[0].Complete = _sURL;
            xTransformer.parseStrict(oURL);
        return oURL[0];
        } catch (Exception e) {
            e.printStackTrace(System.out);
        }
        return null;
    }


    private XFrame getCurrentFrame(){
    try{
        Object oDesktop = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.frame.Desktop", getXComponentContext());
        XDesktop xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, oDesktop);
        return xDesktop.getCurrentFrame();
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}


    public XDispatch getXDispatcher(com.sun.star.util.URL oURL) {
    try {
        com.sun.star.util.URL[] oURLArray = new com.sun.star.util.URL[1];
        oURLArray[0] = oURL;
        XDispatchProvider xDispatchProvider = (XDispatchProvider) UnoRuntime.queryInterface(XDispatchProvider.class, getCurrentFrame());
        XDispatch xDispatch = xDispatchProvider.queryDispatch(oURLArray[0], "_top", FrameSearchFlag.ALL); // "_self"
        return xDispatch;
    } catch (Exception e) {
        e.printStackTrace(System.out);
        return null;
    }}


    private PropertyValue[] loadArgs(String url) {
        PropertyValue pv = new PropertyValue();
        pv.Name = "URL";
        pv.Value = url;
        return new PropertyValue[] {pv};
    }



    protected boolean isFilterApplicable(String _sFilter){
        boolean bFilterDoesApply = true;
        if (_sFilter.length() > 0){
            if (getName().indexOf(_sFilter) == -1){
                bFilterDoesApply = false;
            }
        }
        return bFilterDoesApply;
    }

    public void setVisible(String _sFilter){
        boolean bisVisible = isFilterApplicable(_sFilter);
        super.setVisible(bisVisible);
    }


    public static boolean isValid(Object[] _oObject){
        if (_oObject != null){
            if (_oObject.length > 0){
                return true;
            }
        }
        return false;
    }


    public static boolean isValid(Object _oObject){
        if (_oObject != null){
            return (!AnyConverter.isVoid(_oObject));
        }
        return false;
    }



    public static String getServiceDescription(Object _oUnoObject){
        String sClassName = "";
        XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(XServiceInfo.class, _oUnoObject);
        if (xServiceInfo != null){
            String[] sChildServiceNames = deleteMandatoryServiceNames(xServiceInfo.getSupportedServiceNames());
            if (sChildServiceNames.length > 0){
                sClassName = sChildServiceNames[0];
            }
        }
        return sClassName;
    }



    protected static String[] getMandatoryServiceNames(String _sServiceName){
    String[] sMandatoryServiceNames  = new String[]{};
    try {
        TypeClass[] eTypeClasses = new com.sun.star.uno.TypeClass[1];
        eTypeClasses[0] = com.sun.star.uno.TypeClass.SERVICE;
        XTypeDescriptionEnumeration xTDEnumeration = getXTypeDescriptionEnumerationAccess().createTypeDescriptionEnumeration(Introspector.getModuleName(_sServiceName), eTypeClasses, TypeDescriptionSearchDepth.INFINITE);
        while (xTDEnumeration.hasMoreElements()) {
            XTypeDescription xTD = xTDEnumeration.nextTypeDescription();
            if (xTD.getName().equals(_sServiceName)){
                XServiceTypeDescription xServiceTypeDescription = (XServiceTypeDescription) UnoRuntime.queryInterface(XServiceTypeDescription.class, xTD);
                XServiceTypeDescription[] xMandatoryServiceTypeDescriptions =  xServiceTypeDescription.getMandatoryServices();
                int nlength = xMandatoryServiceTypeDescriptions.length;
                sMandatoryServiceNames  = new String[nlength];
                for (int i = 0; i < nlength; i++){
                    sMandatoryServiceNames[i] = xMandatoryServiceTypeDescriptions[i].getName();
                }

            }
        }
    } catch ( java.lang.Exception e) {
        System.out.println(System.out);
    }
        return sMandatoryServiceNames;
    }


    protected static String[] deleteMandatoryServiceNames(String[] _sServiceNames){
    try{
        List aList = java.util.Arrays.asList(_sServiceNames);
        Vector aVector = new Vector(aList);
        for (int n = 0; n < _sServiceNames.length; n++){
            String[] sDelServiceNames = getMandatoryServiceNames(_sServiceNames[n]);
            for (int m = 0; m < sDelServiceNames.length; m++){
                if (aVector.contains(sDelServiceNames[m])){
                    int nIndex = aVector.indexOf(sDelServiceNames[m]);
                    aVector.remove(nIndex);
                }
            }
        }
        String[] sRetArray = new String[aVector.size()];
        aVector.toArray(sRetArray);
        return sRetArray;
    } catch (java.lang.Exception exception) {
        exception.printStackTrace(System.out);
    }
        return new String[]{};
    }


    protected  static String getDisplayValueOfPrimitiveType(Object _objectElement){
    String sValue ="";
    try{
        if (AnyConverter.isString(_objectElement)){
            sValue = AnyConverter.toString(_objectElement);
        }
        else if (AnyConverter.isBoolean(_objectElement)){
            sValue += AnyConverter.toBoolean(_objectElement);
        }
        else if (AnyConverter.isByte(_objectElement)){
            sValue += AnyConverter.toByte(_objectElement);
        }
        else if (AnyConverter.isChar(_objectElement)){
            sValue += AnyConverter.toChar(_objectElement);
        }
        else if (AnyConverter.isDouble(_objectElement)){
            sValue += AnyConverter.toDouble(_objectElement);
        }
        else if (AnyConverter.isFloat(_objectElement)){
            sValue += AnyConverter.toFloat(_objectElement);
        }
        else if (AnyConverter.isInt(_objectElement)){
            sValue += AnyConverter.toInt(_objectElement);
        }
        else if (AnyConverter.isLong(_objectElement)){
            sValue += AnyConverter.toLong(_objectElement);
        }
        else if (AnyConverter.isShort(_objectElement)){
            sValue += AnyConverter.toShort(_objectElement);
        }
    }
    catch( Exception e ) {
        System.err.println( e );
    }
        return sValue;
    }

    protected static String[] getDisplayValuesofPrimitiveArray(Object _oUnoObject){
    String[] sDisplayValues = null;
    try{
        Type aType = AnyConverter.getType(_oUnoObject);
        TypeClass aTypeClass = aType.getTypeClass();
        int nTypeValue = aTypeClass.getValue();
        if (nTypeValue == TypeClass.SEQUENCE_value){
            nTypeValue = (sequenceComponentType(aType)).getTypeClass().getValue();
        }
        switch (nTypeValue){
            case TypeClass.BOOLEAN_value:
                boolean[] bBooleans = (boolean[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[bBooleans.length];
                for (int i = 0; i < bBooleans.length; i++){
                    sDisplayValues[i] = Boolean.toString(bBooleans[i]);
                }
                break;
            case TypeClass.BYTE_value:
                byte[] bBytes = (byte[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[bBytes.length];
                for (int i = 0; i < bBytes.length; i++){
                    sDisplayValues[i] = "" + bBytes[i];
                }
                break;
            case TypeClass.DOUBLE_value:
                double[] fdoubles = (double[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[fdoubles.length];
                for (int i = 0; i < fdoubles.length; i++){
                    sDisplayValues[i] = String.valueOf(fdoubles[i]);
                }
                break;
            case TypeClass.FLOAT_value:
                float[] ffloats = (float[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[ffloats.length];
                for (int i = 0; i < ffloats.length; i++){
                    sDisplayValues[i] = String.valueOf(ffloats[i]);
                }
                break;
            case TypeClass.LONG_value:
                int[] nints = (int[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[nints.length];
                for (int i = 0; i < nints.length; i++){
                    sDisplayValues[i] = String.valueOf(nints[i]);
                }
                break;
            case TypeClass.HYPER_value:
                long[] nlongs = (long[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[nlongs.length];
                for (int i = 0; i < nlongs.length; i++){
                    sDisplayValues[i] = String.valueOf(nlongs[i]);
                }
                break;
            case TypeClass.SHORT_value:
                short[] nShorts = (short[]) AnyConverter.toArray(_oUnoObject);
                sDisplayValues = new String[nShorts.length];
                for (int i = 0; i < nShorts.length; i++){
                    sDisplayValues[i] = "" +  nShorts[i];
                }
                break;
            case TypeClass.CHAR_value:
                break;
            default:
                System.out.println("Value could not be retrieved: " + aType.getTypeClass().getClass().getName());
            }
        return sDisplayValues;
    }
    catch( Exception e ) {
        System.err.println( e );
        return null;
    }}


    private static Type sequenceComponentType(Type sequenceType) {
//        assert sequenceType.getTypeClass() == TypeClass.SEQUENCE;
        String n = sequenceType.getTypeName();
        final String PREFIX = "[]";
//        assert n.startsWith(PREFIX);
        return new Type(n.substring(PREFIX.length()));
    }


    protected static String getNodeDescription(Object _oUnoObject, int _nIndex){
        return getNodeDescription(_oUnoObject) + "[" + (_nIndex + 1) + "]";
    }


    protected static String getNodeDescription(Object _oUnoObject){
        XServiceInfo xServiceInfo = ( XServiceInfo ) UnoRuntime.queryInterface( XServiceInfo.class, _oUnoObject );
        if ( xServiceInfo != null ) {
            return xServiceInfo.getImplementationName();
        }
        String sClassName = _oUnoObject.getClass().getName();
        if (Introspector.getIntrospector().isObjectPrimitive(_oUnoObject)){         //super.isO{sObjectClassName.equals("java.lang.String"))issClassName.equals("java.lang.String"))
            return _oUnoObject.toString();
        }
        else{
            return _oUnoObject.getClass().getName();
        }
    }

}
