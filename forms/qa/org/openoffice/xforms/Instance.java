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

import com.sun.star.xml.dom.DOMException;
import com.sun.star.xml.dom.XDocument;
import com.sun.star.xml.dom.XNode;
import com.sun.star.xml.dom.XNodeList;
import java.util.NoSuchElementException;

public class Instance
{
    private final Model           m_model;
    private final XDocument       m_domInstance;

    protected Instance( Model _model, XDocument _domInstance )
    {
        m_model = _model;
        m_domInstance = _domInstance;
    }

    /** creates a new element in the instance
     *
     * The element will be inserted immediately below the root node of the instance.
     *
     * @param _elementName the name of the to-be-created element
     * @return the node of the newly created element
     */
    public XNode createElement( String _elementName ) throws DOMException
    {
        return createElement( m_domInstance, _elementName, null );
    }



    /** creates a new element in the instance
     *
     * The element will be inserted immediately below a given XNode.
     *
     * @param _parentElement
     *      the node whose child shall be created
     * @param _elementName
     *      the name of the to-be-created element
     * @param _initialNodeValue
     *      the initial value to set at the node. Might be null, in this case no value is set.
     * @return
     *      the node of the newly created element
     */
    private XNode createElement( XNode _parentElement, String _elementName, String _initialNodeValue ) throws DOMException
    {
        XNode node = _parentElement.appendChild(
            m_model.getUIHelper().createElement( _parentElement, _elementName )
        );
        if ( _initialNodeValue != null )
            node.setNodeValue( _initialNodeValue );
        return node;
    }

    /** removes a child of the root-level node from the instance
     *
     * @param _elementName
     *  the name of the to-be-removed child
     */
    public XNode removeNode( String _elementName ) throws DOMException
    {
        return removeNode( m_domInstance, _elementName );
    }

    /** removes a node from the instance
     *
     * @param _parentElement
     *  the node whose child is to be removed
     * @param _elementName
     *  the name of the to-be-removed child
     */
    private XNode removeNode( XNode _parentElement, String _elementName ) throws DOMException
    {
        XNodeList nodes = _parentElement.getChildNodes();
        for ( int i=0; i<nodes.getLength(); ++i )
        {
            XNode node = nodes.item(i);
            if ( node.getLocalName().equals( _elementName ) )
            {
                _parentElement.removeChild( node );
                return node;
            }
        }
        throw new NoSuchElementException();
    }





    /** creates an attribute for the given node
     *
     * @param _parentElement
     *      the element at which the attribute should be created
     * @param _attribName
     *      the name of the to-be-created attribute
     * @return
     *      the DOM node, which has already been inserted into the DOM tree
     */
    public XNode createAttribute( XNode _parentElement, String _attribName ) throws DOMException
    {
        return createAttribute( _parentElement, _attribName, null );
    }

    /** creates an attribute for the given node
     *
     * @param _parentElement
     *      the element at which the attribute should be created
     * @param _attribName
     *      the name of the to-be-created attribute
     * @param _initialNodeValue
     *      the initial value to set at the node. Might be null, in this case no value is set.
     * @return
     *      the DOM node, which has already been inserted into the DOM tree
     */
    public XNode createAttribute( XNode _parentElement, String _attribName, String _initialNodeValue ) throws DOMException
    {
        XNode node = _parentElement.appendChild(
            m_model.getUIHelper().createAttribute( _parentElement, _attribName )
        );
        if ( _initialNodeValue != null )
            node.setNodeValue( _initialNodeValue );
        return node;
    }
}
