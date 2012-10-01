/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.openoffice.xforms;

import com.sun.star.xml.dom.DOMException;
import com.sun.star.xml.dom.XDocument;
import com.sun.star.xml.dom.XNode;
import com.sun.star.xml.dom.XNodeList;
import java.util.NoSuchElementException;

public class Instance
{
    private Model           m_model;
    private XDocument       m_domInstance;

    protected Instance( Model _model, XDocument _domInstance )
    {
        m_model = _model;
        m_domInstance = _domInstance;
    }

    /** creates a new element in the instance
     *
     * The element will be inserted immediately below the root node of the instance.
     *
     * @param _elementName
     *      the name of the to-be-created element
     * @return
     *      the node of the newly created element
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createElement( String _elementName ) throws DOMException
    {
        return createElement( m_domInstance, _elementName, null );
    }

    /** creates a new element in the instance
     *
     * The element will be inserted immediately below the root node of the instance.
     *
     * @param _elementName
     *      the name of the to-be-created element
     * @param _initialNodeValue
     *      the initial value to set at the node. Might be null, in this case no value is set.
     * @return
     *      the node of the newly created element
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createElement( String _elementName, String _initialNodeValue ) throws DOMException
    {
        return createElement( m_domInstance, _elementName, _initialNodeValue );
    }

    /** creates a new element in the instance
     *
     * The element will be inserted immediately below a given XNode.
     *
     * @param _parentElement
     *      the node whose child shall be created
     * @param _elementName
     *      the name of the to-be-created element
     * @return
     *      the node of the newly created element
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createElement( XNode _parentElement, String _elementName ) throws DOMException
    {
        return createElement( _parentElement, _elementName, null );
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
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createElement( XNode _parentElement, String _elementName, String _initialNodeValue ) throws DOMException
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
    public XNode removeNode( XNode _parentElement, String _elementName ) throws DOMException
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

    /** creates an attribute for the root node of the instance
     *
     * @param _attribName
     *      the name of the to-be-created attribute
     * @return
     *      the DOM node, which has already been inserted into the DOM tree
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createAttribute( String _attribName ) throws DOMException
    {
        return createAttribute( m_domInstance, _attribName, null );
    }

    /** creates an attribute for the root node of the instance
     *
     * @param _attribName
     *      the name of the to-be-created attribute
     * @param _initialNodeValue
     *      the initial value to set at the node. Might be null, in this case no value is set.
     * @return
     *      the DOM node, which has already been inserted into the DOM tree
     * @throws com.sun.star.xml.dom.DOMException
     */
    public XNode createAttribute( String _attribName, String _initialNodeValue ) throws DOMException
    {
        return createAttribute( m_domInstance, _attribName, _initialNodeValue );
    }

    /** creates an attribute for the given node
     *
     * @param _parentElement
     *      the element at which the attribute should be created
     * @param _attribName
     *      the name of the to-be-created attribute
     * @return
     *      the DOM node, which has already been inserted into the DOM tree
     * @throws com.sun.star.xml.dom.DOMException
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
     * @throws com.sun.star.xml.dom.DOMException
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
