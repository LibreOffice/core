/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
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
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class UnoNode{


    Object m_oUnoObject;
    private XMultiComponentFactory m_xMultiComponentFactory;
    private XComponentContext m_xComponentContext;
    private Object[] m_oParamObjects = null;
    private int m_nNodeType = XUnoNode.nOTHERS;
    private Type aType = null;
    private String sLabel = "";



    /** Creates a new instance of UnoNode */
    public UnoNode(Object _oUnoObject) {
        m_xComponentContext = Introspector.getIntrospector().getXComponentContext();
        m_xMultiComponentFactory = m_xComponentContext.getServiceManager();
        m_oUnoObject = _oUnoObject;
    }

    public UnoNode(Object _oUnoObject, Type _aType) {
        this(_oUnoObject);
        aType = _aType;
        m_nNodeType = XUnoNode.nINTERFACE;
    }

    public Object getUnoObject(){
        return m_oUnoObject;
    }


    private XComponentContext getXComponentContext(){
        return m_xComponentContext;
    }


    private XMultiComponentFactory getXMultiComponentFactory(){
        return m_xMultiComponentFactory;
    }


    public String getAnchor(){
        return "";
    }

    public int getNodeType(){
        return m_nNodeType;
    }

    public void setNodeType(int _nNodeType){
        m_nNodeType = _nNodeType;
    }

    public String getClassName(){
        String sClassName = "";
        if (m_nNodeType == XUnoNode.nINTERFACE){
            sClassName = aType.getTypeName();
        }
        else if(m_nNodeType == XUnoNode.nSERVICE){
            sClassName = sLabel;
        }
        return sClassName;
    }


    public Type getUnoType(){
        return aType;
    }

    protected void  setLabel(String _sLabel){
        sLabel = _sLabel;
    }

    public void openIdlDescription(String _sIDLUrl, String _sClassName){
    try{
        String sIDLUrl = _sIDLUrl;
        String sAnchor = ""; // TODO find out how the Anchor may be set at the html file;  //_sAnchor;
        boolean bExists = Introspector.getIntrospector().getXSimpleFileAccess().exists(sIDLUrl);
        if (sIDLUrl.equals("") || (!bExists)){
            sIDLUrl = "https://api.libreoffice.org/" + Inspector.sIDLDOCUMENTSUBFOLDER;
        }
        if (!sIDLUrl.endsWith("/")){
            sIDLUrl += "/";
        }
        if (_sClassName.equals("")){
            sIDLUrl += "index";
            sAnchor = "";
        }
        else{
            sIDLUrl += _sClassName.replace(".", "_1_1");
        }
        sIDLUrl += ".html";
        if (sAnchor != null){
            if (!sAnchor.equals("")){
                sIDLUrl += "#" + sAnchor;
            }
        }
        URL openHyperlink = getDispatchURL(".uno:OpenHyperlink");
        PropertyValue pv = new PropertyValue();
        pv.Name = "URL";
        pv.Value = sIDLUrl;
        getXDispatcher(openHyperlink).dispatch(openHyperlink, new PropertyValue[] {pv});
    } catch(Exception exception) {
        exception.printStackTrace(System.err);
    }}


    private com.sun.star.util.URL getDispatchURL(String _sURL){
        try {
            Object oTransformer = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.util.URLTransformer", getXComponentContext());
            XURLTransformer xTransformer = UnoRuntime.queryInterface(XURLTransformer.class, oTransformer);
            com.sun.star.util.URL[] oURL = new com.sun.star.util.URL[1];
            oURL[0] = new com.sun.star.util.URL();
            oURL[0].Complete = _sURL;
            xTransformer.parseStrict(oURL);
        return oURL[0];
        } catch (Exception e) {
            e.printStackTrace(System.err);
        }
        return null;
    }


    private XFrame getCurrentFrame(){
    try{
        Object oDesktop = getXMultiComponentFactory().createInstanceWithContext("com.sun.star.frame.Desktop", getXComponentContext());
        XDesktop xDesktop = UnoRuntime.queryInterface(XDesktop.class, oDesktop);
        return xDesktop.getCurrentFrame();
    } catch (Exception e) {
        e.printStackTrace(System.err);
        return null;
    }}


    private XDispatch getXDispatcher(com.sun.star.util.URL oURL) {
    try {
        com.sun.star.util.URL[] oURLArray = new com.sun.star.util.URL[1];
        oURLArray[0] = oURL;
        XDispatchProvider xDispatchProvider = UnoRuntime.queryInterface(XDispatchProvider.class, getCurrentFrame());
        XDispatch xDispatch = xDispatchProvider.queryDispatch(oURLArray[0], "_top", FrameSearchFlag.ALL); // "_self"
        return xDispatch;
    } catch (Exception e) {
        e.printStackTrace(System.err);
        return null;
    }}


    public boolean isFilterApplicable(String _sFilter, String _sName){
        boolean bFilterDoesApply = true;
        if (_sFilter.length() > 0){
            if (_sName.indexOf(_sFilter) == -1){
                bFilterDoesApply = false;
            }
        }
        return bFilterDoesApply;
    }


    public  static String getDisplayValueOfPrimitiveType(Object _objectElement){
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
        String n = sequenceType.getTypeName();
        final String PREFIX = "[]";
        return new Type(n.substring(PREFIX.length()));
    }





    public static String getNodeDescription(Object _oUnoObject){
        XServiceInfo xServiceInfo = UnoRuntime.queryInterface( XServiceInfo.class, _oUnoObject );
        if ( xServiceInfo != null ) {
            return xServiceInfo.getImplementationName();
        }
        if (Introspector.isObjectPrimitive(_oUnoObject)){         //super.isO{sObjectClassName.equals("String"))issClassName.equals("String"))
            return _oUnoObject.toString();
        }
        else{
            return _oUnoObject.getClass().getName();
        }
    }

    public void setParameterObjects(Object[] _oParamObjects){
        m_oParamObjects = _oParamObjects;
    }

    public Object[] getParameterObjects(){
        return m_oParamObjects;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
