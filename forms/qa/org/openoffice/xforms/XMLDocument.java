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

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.xforms.XFormsSupplier;
import com.sun.star.xforms.XFormsUIHelper1;
import com.sun.star.xforms.XModel;
import integration.forms.DocumentType;

public class XMLDocument extends integration.forms.DocumentHelper
{
    private XNameContainer  m_forms;

    /* ------------------------------------------------------------------ */
    public XMLDocument( XMultiServiceFactory _orb ) throws Exception
    {
        super( _orb, implLoadAsComponent( _orb, getDocumentFactoryURL( DocumentType.XMLFORM ) ) );
        impl_initialize( getDocument() );
    }

    /* ------------------------------------------------------------------ */
    private void impl_initialize( XComponent _document )
    {
        XFormsSupplier  formsSupplier = UnoRuntime.queryInterface( XFormsSupplier.class,
            _document );

        if ( formsSupplier == null )
            throw new IllegalArgumentException();

        m_forms = formsSupplier.getXForms();
    }

    /* ------------------------------------------------------------------ */
    public String[] getXFormModelNames()
    {
        return m_forms.getElementNames();
    }

    /* ------------------------------------------------------------------ */
    public Model getXFormModel( String _modelName ) throws NoSuchElementException
    {
        try
        {
            return new Model(m_forms.getByName(_modelName));
        }
        catch (WrappedTargetException ex)
        {
            throw new NoSuchElementException(ex);
        }
    }

    /* ------------------------------------------------------------------ */
    public Model addXFormModel( String _modelName )
    {
        XModel newModel = null;
        try
        {
            newModel = UnoRuntime.queryInterface( XModel.class,
                getOrb().createInstance( "com.sun.star.xforms.Model" ) );
            newModel.setID(_modelName);
            XFormsUIHelper1 modelHelper = UnoRuntime.queryInterface(
                XFormsUIHelper1.class, newModel );
            modelHelper.newInstance( "Instance 1", "", true );
            newModel.initialize();

            m_forms.insertByName(_modelName, newModel);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
        return new Model( newModel );
    }
}
