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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_MODEL_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_MODEL_HXX

#include <cppuhelper/implbase.hxx>
#include "propertysetbase.hxx"
#include <com/sun/star/xforms/XModel2.hpp>
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include "mip.hxx"
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
typedef cppu::ImplInheritanceHelper<
    PropertySetBase,
    css::xforms::XModel2,
    css::xforms::XFormsUIHelper1,
    css::util::XUpdatable,
    css::lang::XUnoTunnel,
    css::lang::XServiceInfo
> Model_t;
class Model : public Model_t
{
    // a number of local typedefs, to make the remaining header readable
    typedef css::uno::Reference<css::xml::dom::XNode> XNode_t;
    typedef css::uno::Reference<css::beans::XPropertySet> XPropertySet_t;

    typedef std::multimap<XNode_t,std::pair<void*,MIP> > MIPs_t;


private:

    OUString msID;                     /// the model ID
    rtl::Reference<BindingCollection>    mxBindings;     /// the bindings
    rtl::Reference<SubmissionCollection> mxSubmissions;  /// the submissions
    rtl::Reference<InstanceCollection>   mxInstances;    /// the instance(s)

    css::uno::Reference<css::xforms::XDataTypeRepository> mxDataTypes;      /// the XSD data-types used
    css::uno::Reference<css::xml::dom::XDocument> mxForeignSchema;            /// the XSD-schema part we cannot
                                            /// map onto data types
    OUString msSchemaRef;              /// xforms:model/@schema attribute

    css::uno::Reference<css::container::XNameContainer> mxNamespaces;          /// namespaces for entire model

    MIPs_t maMIPs;                          /// map nodes to their MIPs

    bool mbInitialized;                     /// has model been initialized ?
    bool mbExternalData;                    /// is the data of this model to be considered an integral part of the document?

    void initializePropertySet();

    void ensureAtLeastOneInstance();


public:

    /// create a new model with an empty, default instance
    Model();
    virtual ~Model() throw() override;

    // get Model implementation from API object
    static Model* getModel( const css::uno::Reference<css::xforms::XModel>& );

    xforms::EvaluationContext getEvaluationContext();


    static css::uno::Sequence<sal_Int8> getUnoTunnelID();


    // get/set that part of the schema, that we can't interpret as data types
    css::uno::Reference<css::xml::dom::XDocument> getForeignSchema() const { return mxForeignSchema;}
    void setForeignSchema( const css::uno::Reference<css::xml::dom::XDocument>& );

    // get/set the xforms:model/@schema attribute
    OUString getSchemaRef() const { return msSchemaRef;}
    void setSchemaRef( const OUString& );

    // get/set namespaces for entire model
    css::uno::Reference<css::container::XNameContainer> getNamespaces() const { return mxNamespaces;}
    void setNamespaces( const css::uno::Reference<css::container::XNameContainer>& );

    // get/set the ExternalData property
    bool getExternalData() const { return mbExternalData;}
    void setExternalData( bool _bData );


#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    void dbg_assertInvariant() const;
#endif


    // MIP (model item property) management


    // register MIPs which apply to a given node; only to be called by bindings
    // (The pTag parameter serves only to be able to remove the MIPs
    // that were added using the same tag. No functions will be
    // performed on it; hence the void* type.)
    void addMIP( void* pTag, const XNode_t&, const MIP& );
    void removeMIPs( void const * pTag );

    /// query which MIPs apply to the given node
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
    bool isInitialized() const { return mbInitialized;}

    /// is model currently valid (for submission)?
    bool isValid() const;


    // XModel
    //    implement the xforms::XModel implementation


    virtual OUString SAL_CALL getID() override;

    virtual void SAL_CALL setID( const OUString& sID ) override;

    virtual void SAL_CALL initialize() override;

    virtual void SAL_CALL rebuild() override;

    virtual void SAL_CALL recalculate() override;

    virtual void SAL_CALL revalidate() override;

    virtual void SAL_CALL refresh() override;

    virtual void SAL_CALL submit( const OUString& sID ) override;

    virtual void SAL_CALL submitWithInteraction( const OUString& id, const css::uno::Reference<css::task::XInteractionHandler>& _rxHandler ) override;

    virtual css::uno::Reference<css::xforms::XDataTypeRepository> SAL_CALL getDataTypeRepository(  ) override;


    // XModel: instance management

    virtual css::uno::Reference<css::container::XSet> SAL_CALL getInstances() override;

    virtual css::uno::Reference<css::xml::dom::XDocument> SAL_CALL getInstanceDocument( const OUString& ) override;

    virtual css::uno::Reference<css::xml::dom::XDocument> SAL_CALL getDefaultInstance() override;


    // XModel: binding management

    virtual css::uno::Reference<css::beans::XPropertySet> SAL_CALL createBinding() override;

    virtual css::uno::Reference<css::beans::XPropertySet> SAL_CALL cloneBinding( const css::uno::Reference<css::beans::XPropertySet>& ) override;

    virtual css::uno::Reference<css::beans::XPropertySet> SAL_CALL getBinding( const OUString& ) override;

    virtual css::uno::Reference<css::container::XSet> SAL_CALL getBindings() override;


    // XModel: submission management

    virtual css::uno::Reference<css::xforms::XSubmission> SAL_CALL createSubmission() override;

    virtual css::uno::Reference<css::xforms::XSubmission> SAL_CALL cloneSubmission( const css::uno::Reference<css::beans::XPropertySet>& ) override;

    virtual css::uno::Reference<css::xforms::XSubmission> SAL_CALL getSubmission( const OUString& ) override;

    virtual css::uno::Reference<css::container::XSet> SAL_CALL getSubmissions() override;

    // XPropertySet

    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& p) override
        { return PropertySetBase::getPropertyValue(p); }

    virtual void SAL_CALL addPropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { PropertySetBase::addPropertyChangeListener(p1, p2); }

    virtual void SAL_CALL removePropertyChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XPropertyChangeListener>& p2) override
        { PropertySetBase::removePropertyChangeListener(p1, p2); }

    virtual void SAL_CALL addVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { PropertySetBase::addVetoableChangeListener(p1, p2); }

    virtual void SAL_CALL removeVetoableChangeListener(const OUString& p1, const css::uno::Reference<css::beans::XVetoableChangeListener>& p2) override
        { PropertySetBase::removeVetoableChangeListener(p1, p2); }

    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
        { return PropertySetBase::getPropertySetInfo(); }

   virtual void SAL_CALL setPropertyValue(const OUString& p1, const css::uno::Any& p2) override
        { PropertySetBase::setPropertyValue(p1, p2); }


    // XFormsUIHelper1 & friends:
    //   (implementation in model_ui.cxx)


    /// determine a reasonable control service for a given node
    /// (based on data type MIP assigned to the node)
    virtual OUString SAL_CALL getDefaultServiceNameForNode( const css::uno::Reference<css::xml::dom::XNode>& xNode ) override;

    /// call getDefaultBindingExpressionForNode with default evaluation context
    virtual OUString SAL_CALL getDefaultBindingExpressionForNode( const css::uno::Reference<css::xml::dom::XNode>& xNode ) override;

    /// determine a reasonable default binding expression for a given node
    /// and a given evaluation context
    /// @returns expression, or empty string if no expression could be derived
    OUString getDefaultBindingExpressionForNode(
        const XNode_t&,
        const EvaluationContext& );

    virtual OUString SAL_CALL getNodeDisplayName( const css::uno::Reference<css::xml::dom::XNode>&,
                                                       sal_Bool bDetail ) override;

    virtual OUString SAL_CALL getNodeName( const css::uno::Reference<css::xml::dom::XNode>& ) override;

    virtual OUString SAL_CALL getBindingName( const css::uno::Reference< ::css::beans::XPropertySet >&,
                                                   sal_Bool bDetail ) override;

    virtual OUString SAL_CALL getSubmissionName( const css::uno::Reference< ::css::beans::XPropertySet >&,
                                                      sal_Bool bDetail ) override;

    virtual css::uno::Reference< ::css::beans::XPropertySet > SAL_CALL cloneBindingAsGhost( const css::uno::Reference< ::css::beans::XPropertySet >& ) override;

    virtual void SAL_CALL removeBindingIfUseless( const css::uno::Reference< ::css::beans::XPropertySet >& ) override;

    virtual css::uno::Reference<css::xml::dom::XDocument> SAL_CALL newInstance( const OUString& sName,
                                              const OUString& sURL,
                                              sal_Bool bURLOnce ) override;

    virtual void SAL_CALL renameInstance( const OUString& sFrom,
                                          const OUString& sTo,
                                          const OUString& sURL,
                                          sal_Bool bURLOnce ) override;

    virtual void SAL_CALL removeInstance( const OUString& sName ) override;


    virtual css::uno::Reference<css::xforms::XModel> SAL_CALL newModel( const css::uno::Reference<css::frame::XModel>& xComponent,
                                        const OUString& sName ) override;
    virtual void SAL_CALL renameModel( const css::uno::Reference<css::frame::XModel>& xComponent,
                                       const OUString& sFrom,
                                       const OUString& sTo ) override;

    virtual void SAL_CALL removeModel( const css::uno::Reference<css::frame::XModel>& xComponent,
                                       const OUString& sName ) override;


    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL createElement(
                                            const css::uno::Reference< ::css::xml::dom::XNode >& xParent,
                                            const OUString& sName ) override;

    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL createAttribute(
                                              const css::uno::Reference< ::css::xml::dom::XNode >& xParent,
                                              const OUString& sName ) override;

    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL renameNode(
                                         const css::uno::Reference< ::css::xml::dom::XNode >& xNode,
                                         const OUString& sName ) override;

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getBindingForNode( const
                                         css::uno::Reference<css::xml::dom::XNode>&,
                                         sal_Bool bCreate ) override;

    virtual void SAL_CALL removeBindingForNode( const css::uno::Reference< ::css::xml::dom::XNode >& ) override;

    virtual OUString SAL_CALL getResultForExpression(
        const css::uno::Reference< css::beans::XPropertySet >& xBinding,
        sal_Bool bIsBindingExpression,
        const OUString& sExpression ) override;

    virtual sal_Bool SAL_CALL isValidXMLName( const OUString& sName ) override;

    virtual sal_Bool SAL_CALL isValidPrefixName( const OUString& sName ) override;

    virtual void SAL_CALL setNodeValue(
        const css::uno::Reference< ::css::xml::dom::XNode >& xNode,
        const OUString& sValue ) override;


    // XUpdatable


public:
    virtual void SAL_CALL update() override;


    // XUnoTunnel


public:
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<sal_Int8>& ) override;


    // XTypeProvider::getImplementationId


public:
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};

} // namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
