/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _MODEL_HXX
#define _MODEL_HXX

#include <cppuhelper/implbase4.hxx>
#include <propertysetbase.hxx>
#include <com/sun/star/xforms/XModel2.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <map>


// forward declaractions
namespace com { namespace sun { namespace star
{
    namespace xml { namespace dom { class XDocument; } }
    namespace xml { namespace dom { class XNode; } }
    namespace uno { template<typename T> class Sequence; }
    namespace lang { class IndexOutOfBoundsException; }
    namespace lang { class IllegalArgumentException; }
    namespace beans { class XPropertySet; }
    namespace container { class XSet; }
    namespace container { class XNameContainer; }
    namespace frame { class XModel; }
} } }
namespace xforms
{
    class MIP;
    class BindingCollection;
    class SubmissionCollection;
    class InstanceCollection;
    class EvaluationContext;
}


namespace xforms
{

/** An XForms Model. Contains:
 *  # (set of) instance data (XML DOM tree)
 *  # (set of) bindings
 *  # (set of) submissions
 *  # (NOT YET IMPLEMENTED) actions (set of)
 *
 * See http://www.w3.org/TR/xforms/ for more information.
 */
typedef cppu::ImplInheritanceHelper4<
    PropertySetBase,
    com::sun::star::xforms::XModel2,
    com::sun::star::xforms::XFormsUIHelper1,
    com::sun::star::util::XUpdatable,
    com::sun::star::lang::XUnoTunnel
> Model_t;
class Model : public Model_t
{
    // a number of local typedefs, to make the remaining header readable
    typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocument> XDocument_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> XNode_t;
    typedef com::sun::star::lang::IndexOutOfBoundsException IndexOutOfBoundsException_t;
    typedef com::sun::star::lang::IllegalArgumentException IllegalArgumentException_t;
    typedef com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> XPropertySet_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository> XDataTypeRepository_t;
    typedef com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> XNameContainer_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xforms::XSubmission> XSubmission_t;
    typedef com::sun::star::uno::Reference<com::sun::star::frame::XModel> Frame_XModel_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xforms::XModel> XModel_t;
    typedef com::sun::star::uno::Reference<com::sun::star::task::XInteractionHandler> XInteractionHandler_t;

    typedef com::sun::star::uno::Reference<com::sun::star::container::XSet> XSet_t;
    typedef com::sun::star::beans::PropertyVetoException PropertyVetoException_t;
    typedef com::sun::star::beans::UnknownPropertyException UnknownPropertyException_t;
    typedef com::sun::star::util::VetoException VetoException_t;
    typedef com::sun::star::lang::WrappedTargetException WrappedTargetException_t;
    typedef com::sun::star::uno::RuntimeException RuntimeException_t;
    typedef com::sun::star::uno::Any Any_t;
    typedef com::sun::star::uno::Sequence<sal_Int8> IntSequence_t;
    typedef std::multimap<XNode_t,std::pair<void*,MIP> > MIPs_t;


private:

    OUString msID;                     /// the model ID
    BindingCollection* mpBindings;          /// the bindings
    SubmissionCollection* mpSubmissions;    /// the submissions
    InstanceCollection* mpInstances;        /// the instance(s)

    XDataTypeRepository_t mxDataTypes;      /// the XSD data-types used
    XDocument_t mxForeignSchema;            /// the XSD-schema part we cannot
                                            /// map onto data types
    OUString msSchemaRef;              /// xforms:model/@schema attribute

    XNameContainer_t mxNamespaces;          /// namespaces for entire model


    // references to mpBindings/mpSubmissions, for UNO reference counting
    XSet_t mxBindings;
    XSet_t mxSubmissions;
    XSet_t mxInstances;

    MIPs_t maMIPs;                          /// map nodes to their MIPs

    bool mbInitialized;                     /// has model been initialized ?
    bool mbExternalData;                    /// is the data of this model to be considered an ingegral part of the document?

    void initializePropertySet();

    void ensureAtLeastOneInstance();


public:

    /// create a new model with an empty, default instance
    Model();
    virtual ~Model() throw();

    // get Model implementation from API object
    static Model* getModel( const com::sun::star::uno::Reference<com::sun::star::xforms::XModel>& );

    xforms::EvaluationContext getEvaluationContext();


    static IntSequence_t getUnoTunnelID();


    // get/set that part of the schema, that we can't interpret as data types
    XDocument_t getForeignSchema() const;
    void setForeignSchema( const XDocument_t& );

    // get/set the xforms:model/@schema attribute
    OUString getSchemaRef() const;
    void setSchemaRef( const OUString& );

    // get/set namespaces for entire model
    XNameContainer_t getNamespaces() const;
    void setNamespaces( const XNameContainer_t& );

    // get/set the ExternalData property
    bool getExternalData() const;
    void setExternalData( bool _bData );


#if OSL_DEBUG_LEVEL > 1
    void dbg_assertInvariant() const;
#endif


    //
    // MIP (model item property) management
    //

    // register MIPs which apply to a given node; only to be called by bindings
    // (The pTag parameter serves only to be able to remove the MIPs
    // that were added using the same tag. No functions will be
    // performed on it; hence the void* type.)
    void addMIP( void* pTag, const XNode_t&, const MIP& );
    void removeMIPs( void* pTag );

    /// query which MIPs appy to the given node
    MIP queryMIP( const XNode_t& xNode ) const;

    /// re-bind all bindings
    void rebind();

    /// call defer notifications on all bindings
    void deferNotifications( bool );

    /// set a data value in the instance
    /// (also defers notifications)
    bool setSimpleContent( const XNode_t&, const OUString& );

    /// load instance data
    void loadInstance( sal_Int32 nInstance );
    void loadInstances();

    /// has model been initialized?
    bool isInitialized() const;

    /// is model currently valid (for submission)?
    bool isValid() const;



    //
    // XModel
    //    implement the xforms::XModel implementation
    //


    virtual OUString SAL_CALL getID()
        throw( RuntimeException_t );

    virtual void SAL_CALL setID( const OUString& sID )
        throw( RuntimeException_t );

    virtual void SAL_CALL initialize()
        throw( RuntimeException_t );

    virtual void SAL_CALL rebuild()
        throw( RuntimeException_t );

    virtual void SAL_CALL recalculate()
        throw( RuntimeException_t );

    virtual void SAL_CALL revalidate()
        throw( RuntimeException_t );

    virtual void SAL_CALL refresh()
        throw( RuntimeException_t );

    virtual void SAL_CALL submit( const OUString& sID )
        throw( VetoException_t, WrappedTargetException_t, RuntimeException_t );

    virtual void SAL_CALL submitWithInteraction( const OUString& id, const XInteractionHandler_t& _rxHandler )
        throw( VetoException_t, WrappedTargetException_t, RuntimeException_t );

    virtual XDataTypeRepository_t SAL_CALL getDataTypeRepository(  )
        throw( RuntimeException_t );


    // XModel: instance management

    virtual XSet_t SAL_CALL getInstances()
        throw( RuntimeException_t );

    virtual XDocument_t SAL_CALL getInstanceDocument( const OUString& )
        throw( RuntimeException_t );

    virtual XDocument_t SAL_CALL getDefaultInstance()
        throw( RuntimeException_t );



    // XModel: binding management

    virtual XPropertySet_t SAL_CALL createBinding()
        throw( RuntimeException_t );

    virtual XPropertySet_t SAL_CALL cloneBinding( const XPropertySet_t& )
        throw( RuntimeException_t );

    virtual XPropertySet_t SAL_CALL getBinding( const OUString& )
        throw( RuntimeException_t );

    virtual XSet_t SAL_CALL getBindings()
        throw( RuntimeException_t );


    // XModel: submission management

    virtual XSubmission_t SAL_CALL createSubmission()
        throw( RuntimeException_t );

    virtual XSubmission_t SAL_CALL cloneSubmission( const XPropertySet_t& )
        throw( RuntimeException_t );

    virtual XSubmission_t SAL_CALL getSubmission( const OUString& )
        throw( RuntimeException_t );

    virtual XSet_t SAL_CALL getSubmissions()
        throw( RuntimeException_t );

    // XPropertySet

    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p)
        throw( css::uno::RuntimeException )
        { return PropertySetBase::getPropertyValue(p); }

    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2)
        throw( css::uno::RuntimeException )
        { PropertySetBase::addPropertyChangeListener(p1, p2); }

    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2)
        throw( css::uno::RuntimeException )
        { PropertySetBase::removePropertyChangeListener(p1, p2); }

    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2)
        throw( css::uno::RuntimeException )
        { PropertySetBase::addVetoableChangeListener(p1, p2); }

    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2)
        throw( css::uno::RuntimeException )
        { PropertySetBase::removeVetoableChangeListener(p1, p2); }

    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo()
        throw( css::uno::RuntimeException )
        { return PropertySetBase::getPropertySetInfo(); }

   virtual void SAL_CALL setPropertyValue(const OUString& p1, const com::sun::star::uno::Any& p2)
        throw( css::uno::RuntimeException )
        { PropertySetBase::setPropertyValue(p1, p2); }

    //
    // XFormsUIHelper1 & friends:
    //   (implementation in model_ui.cxx)
    //

    /// determine a reasonable control service for a given node
    /// (based on data type MIP assigned to the node)
    virtual OUString SAL_CALL getDefaultServiceNameForNode( const XNode_t& xNode ) throw (RuntimeException_t);

    /// call getDefaultBindingExpressionForNode with default evaluation context
    virtual OUString SAL_CALL getDefaultBindingExpressionForNode( const XNode_t& xNode ) throw (RuntimeException_t);

    /// determine a reasonable default binding expression for a given node
    /// and a given evaluation context
    /// @returns expression, or empty string if no expression could be derived
    OUString getDefaultBindingExpressionForNode(
        const XNode_t&,
        const EvaluationContext& );

    virtual OUString SAL_CALL getNodeDisplayName( const XNode_t&,
                                                       sal_Bool bDetail )
        throw( RuntimeException_t );

    virtual OUString SAL_CALL getNodeName( const XNode_t& )
        throw( RuntimeException_t );

    virtual OUString SAL_CALL getBindingName( const XPropertySet_t&,
                                                   sal_Bool bDetail )
        throw( RuntimeException_t );

    virtual OUString SAL_CALL getSubmissionName( const XPropertySet_t&,
                                                      sal_Bool bDetail )
        throw( RuntimeException_t );

    virtual XPropertySet_t SAL_CALL cloneBindingAsGhost( const XPropertySet_t& )
        throw( RuntimeException_t );

    virtual void SAL_CALL removeBindingIfUseless( const XPropertySet_t& )
        throw( RuntimeException_t );

    virtual XDocument_t SAL_CALL newInstance( const OUString& sName,
                                              const OUString& sURL,
                                              sal_Bool bURLOnce )
        throw( RuntimeException_t );

    virtual void SAL_CALL renameInstance( const OUString& sFrom,
                                          const OUString& sTo,
                                          const OUString& sURL,
                                          sal_Bool bURLOnce )
        throw( RuntimeException_t );

    virtual void SAL_CALL removeInstance( const OUString& sName )
        throw( RuntimeException_t );


    virtual XModel_t SAL_CALL newModel( const Frame_XModel_t& xComponent,
                                        const OUString& sName )
        throw( RuntimeException_t );
    virtual void SAL_CALL renameModel( const Frame_XModel_t& xComponent,
                                       const OUString& sFrom,
                                       const OUString& sTo )
        throw( RuntimeException_t );

    virtual void SAL_CALL removeModel( const Frame_XModel_t& xComponent,
                                       const OUString& sName )
        throw( RuntimeException_t );


    virtual XNode_t SAL_CALL createElement( const XNode_t& xParent,
                                            const OUString& sName )
        throw( RuntimeException_t );

    virtual XNode_t SAL_CALL createAttribute( const XNode_t& xParent,
                                              const OUString& sName )
        throw( RuntimeException_t );

    virtual XNode_t SAL_CALL renameNode( const XNode_t& xNode,
                                         const OUString& sName )
        throw( RuntimeException_t );

    virtual XPropertySet_t SAL_CALL getBindingForNode( const XNode_t&,
                                                       sal_Bool bCreate )
        throw( RuntimeException_t );

    virtual void SAL_CALL removeBindingForNode( const XNode_t& )
        throw( RuntimeException_t );

    virtual OUString SAL_CALL getResultForExpression(
        const XPropertySet_t& xBinding,
        sal_Bool bIsBindingExpression,
        const OUString& sExpression )
        throw( RuntimeException_t );

    virtual sal_Bool SAL_CALL isValidXMLName( const OUString& sName )
        throw( RuntimeException_t );

    virtual sal_Bool SAL_CALL isValidPrefixName( const OUString& sName )
        throw( RuntimeException_t );

    virtual void SAL_CALL setNodeValue(
        const XNode_t& xNode,
        const OUString& sValue )
        throw( RuntimeException_t );


    //
    // XUpdatable
    //

public:
    virtual void SAL_CALL update()
        throw( RuntimeException_t );

    //
    // XUnoTunnel
    //

public:
    virtual sal_Int64 SAL_CALL getSomething( const IntSequence_t& )
        throw( RuntimeException_t );

    //
    // XTypeProvider::getImplementationId
    //

public:
    virtual IntSequence_t SAL_CALL getImplementationId()
        throw( RuntimeException_t );

};

} // namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
