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

package integration.extensions;

import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.inspection.*;
import com.sun.star.frame.*;
import com.sun.star.lang.XServiceInfo;

/**
 *
 * @author fs93730
 */
public class ServicesHandler implements XPropertyHandler
{
    private XComponentContext   m_context;
    private String[]            m_supportedServices;

    private class ClickHandler implements com.sun.star.awt.XActionListener
    {
        XComponentContext   m_context;
        private String      m_serviceName;

        public ClickHandler( XComponentContext _context, String _serviceName )
        {
            m_context = _context;
            m_serviceName = _serviceName;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject)
        {
            // not interested in
        }

        public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
        {
            try
            {
                // translate the service name into a URL to dispatch
                String documentationURL = "http://api.libreoffice.org/docs/common/ref/" + m_serviceName.replace('.','/') + ".html";
                System.out.println( documentationURL );

                // the OpenHyperlink command, to be dispatched to the Desktop
                com.sun.star.util.URL dispatchURL[] = { new com.sun.star.util.URL() };
                dispatchURL[0].Complete = ".uno:OpenHyperlink";
                com.sun.star.util.XURLTransformer transformer = (com.sun.star.util.XURLTransformer)UnoRuntime.queryInterface(
                        com.sun.star.util.XURLTransformer.class,
                        m_context.getServiceManager().createInstanceWithContext( "com.sun.star.util.URLTransformer", m_context ) );
                transformer.parseStrict( dispatchURL );

                // the dispatcher for the OpenHyperlink command
                Frame desktop = new Frame(
                        m_context.getServiceManager().createInstanceWithContext( "com.sun.star.frame.Desktop", m_context ) );
                XDispatch dispatcher = desktop.queryDispatch(dispatchURL[0],"",0);

                // the arguments for the OpenHyperlink command
                PropertyValue dispatchArgs[] = new PropertyValue[] { new PropertyValue() };
                dispatchArgs[0].Name = "URL";
                dispatchArgs[0].Value = documentationURL;

                dispatcher.dispatch(dispatchURL[0], dispatchArgs );
            }
            catch( com.sun.star.uno.Exception e )
            {
                e.printStackTrace( System.err );
            }
        }
    }

    /** Creates a new instance of ServicesHandler */
    public ServicesHandler( XComponentContext _context )
    {
        m_context = _context;
        m_supportedServices = new String[] { };
    }

    public void actuatingPropertyChanged(String _propertyName, Object _newValue, Object _oldValue, com.sun.star.inspection.XObjectInspectorUI _objectInspectorUI, boolean _firstTimeInit) throws com.sun.star.lang.NullPointerException
    {
        // not interested in
    }

    public void addEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        // ingnoring this
    }

    public void addPropertyChangeListener(com.sun.star.beans.XPropertyChangeListener _propertyChangeListener) throws com.sun.star.lang.NullPointerException
    {
        // ingnoring this
    }

    public Object convertToControlValue(String _propertyName, Object _propertyValue, com.sun.star.uno.Type type) throws com.sun.star.beans.UnknownPropertyException
    {
        return _propertyValue;
    }

    public Object convertToPropertyValue(String _propertyName, Object _controlValue) throws com.sun.star.beans.UnknownPropertyException
    {
        return _controlValue;
    }

    public com.sun.star.inspection.LineDescriptor describePropertyLine(String _propertyName, com.sun.star.inspection.XPropertyControlFactory _propertyControlFactory) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.NullPointerException
    {
        com.sun.star.inspection.LineDescriptor descriptor = new com.sun.star.inspection.LineDescriptor();

        descriptor = new LineDescriptor();
        descriptor.Category = "Services";
        descriptor.DisplayName = "supports service";
        descriptor.HasPrimaryButton = descriptor.HasSecondaryButton = false;
        descriptor.IndentLevel = 0;
        try
        {
            XHyperlinkControl hyperlinkControl = (XHyperlinkControl)UnoRuntime.queryInterface(
                    XHyperlinkControl.class, _propertyControlFactory.createPropertyControl( PropertyControlType.HyperlinkField, true ) );
            hyperlinkControl.addActionListener( new ClickHandler( m_context,  _propertyName ) );

            descriptor.Control = hyperlinkControl;
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
        }
        return descriptor;
    }

    public void dispose()
    {
        // nothing to do
    }

    public String[] getActuatingProperties()
    {
        // none
        return new String[] { };
    }

    public com.sun.star.beans.PropertyState getPropertyState(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        return com.sun.star.beans.PropertyState.DIRECT_VALUE;
    }

    public Object getPropertyValue(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        return _propertyName;
    }

    public String[] getSupersededProperties()
    {
        return new String[] { "SupportedServiceNames" };
        // we're used in conjunction with a GenericPropertyHandler, which (via inspection) finds
        // a property SupportedServiceNames, resulting from the XServiceInfo.getSupportedServiceNames
        // method. Since we handle those ourself, we supersede them.
    }

    public com.sun.star.beans.Property[] getSupportedProperties()
    {
        Property[] properties = new Property[ m_supportedServices.length ];
        for ( int i=0; i<m_supportedServices.length; ++i )
            properties[i] = new Property( m_supportedServices[i], 0, new Type( String.class ), (short)0 );
        return properties;
    }

    public void inspect(Object _component) throws com.sun.star.lang.NullPointerException
    {
        XServiceInfo serviceInfo = (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class, _component );
        if ( serviceInfo != null )
            m_supportedServices = serviceInfo.getSupportedServiceNames();
    }

    public boolean isComposable(String _propertyName) throws com.sun.star.beans.UnknownPropertyException
    {
        return true;
    }

    public com.sun.star.inspection.InteractiveSelectionResult onInteractivePropertySelection(String str, boolean param, Object[] obj, com.sun.star.inspection.XObjectInspectorUI xObjectInspectorUI) throws com.sun.star.beans.UnknownPropertyException, com.sun.star.lang.NullPointerException
    {
        return InteractiveSelectionResult.Cancelled;
    }

    public void removeEventListener(com.sun.star.lang.XEventListener _eventListener)
    {
        // ignoring this
    }

    public void removePropertyChangeListener(com.sun.star.beans.XPropertyChangeListener _propertyChangeListener)
    {
        // ignoring this
    }

    public void setPropertyValue(String str, Object obj) throws com.sun.star.beans.UnknownPropertyException
    {
        // we declared our properties as readonly
        throw new java.lang.RuntimeException();
    }

    public boolean suspend(boolean param)
    {
        return true;
    }
}
