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

package org.openoffice.xforms;

import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xforms.XFormsUIHelper1;
import com.sun.star.xforms.XModel;
import com.sun.star.xml.dom.XNode;

public class Model
{
    private final XModel          m_model;
    private final XPropertySet    m_modelProps;
    private final XFormsUIHelper1 m_helper;

    protected Model( Object _model )
    {
        m_model = UnoRuntime.queryInterface( XModel.class, _model );
        m_modelProps = UnoRuntime.queryInterface( XPropertySet.class, _model );
        m_helper = UnoRuntime.queryInterface( XFormsUIHelper1.class,
            m_model );
    }



    protected XFormsUIHelper1 getUIHelper()
    {
        return m_helper;
    }

    public Instance getDefaultInstance()
    {
        return new Instance( this, m_model.getDefaultInstance() );
    }

    /** creates a binding for the given DOM node
     *
     * @param _node     the DOM node to create a binding for
     * @param _dataTypeClass the data type to be used for the binding
     */
    public XPropertySet createBindingForNode( XNode _node, short _dataTypeClass )
    {
        XPropertySet binding = m_helper.getBindingForNode(_node, true);
        try
        {
            String basicTypeName = (String)m_model.getDataTypeRepository().getBasicDataType( _dataTypeClass ).
                getPropertyValue( "Name" );
            binding.setPropertyValue( "Type", basicTypeName );
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        return binding;
    }

    public void setIsDocumentInternalData( boolean _internalData )
    {
        try
        {
            m_modelProps.setPropertyValue("ExternalData", Boolean.valueOf(!_internalData));
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public boolean getIsDocumentInternalData()
    {
        boolean isInternalData = false;
        try
        {
            isInternalData = !((Boolean)m_modelProps.getPropertyValue( "ExternalData" )).booleanValue();
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        return isInternalData;
    }
}
