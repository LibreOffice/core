/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatedata.hxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_UPDATEDATA_HXX
#define CONFIGMGR_BACKEND_UPDATEDATA_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include "utility.hxx"

#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif
#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    class XLayerHandler;
} } } } }
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno = ::com::sun::star::uno;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------
        class ElementUpdate;
        class NodeUpdate;
        class NodeModification;
        class NodeReplace;
        class NodeDrop;
        class PropertyUpdate;
        class PropertyAdd;
        class PropertyReset;

// -----------------------------------------------------------------------------

        class NamedUpdate : public salhelper::SimpleReferenceObject
        {
            rtl::OUString const  m_aName;

        protected:
            explicit
            NamedUpdate(rtl::OUString const & _aName)
            : m_aName(_aName)
            {}

            ~NamedUpdate() {};

        public:
            rtl::OUString const & getName() const { return m_aName; }
        };
// -----------------------------------------------------------------------------
        class ElementUpdate : public NamedUpdate
        {
            NodeUpdate *    m_pParent;
            sal_Int16       m_nFlags;
            sal_Int16       m_nFlagsMask;
        protected:
            ElementUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask);

        public:
            virtual NodeUpdate      * asNodeUpdate(bool _bMerged = false);
            virtual PropertyUpdate  * asPropertyUpdate();

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer) = 0;

        public:
            NodeUpdate * getParent() const { return m_pParent; }

            sal_Int16 changedFlags() const { return m_nFlagsMask; }
            sal_Int16 updateFlags(sal_Int16 _nFlags = 0) const;

        };
// -----------------------------------------------------------------------------

        class NodeUpdate : public ElementUpdate
        {
            typedef std::map< rtl::OUString, rtl::Reference<ElementUpdate> > ElementList;
        public:
            enum Op { modify, reset, replace };

        protected:
            NodeUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, Op _op);

            virtual NodeUpdate * asNodeUpdate(bool _bMerged);

        public:
            bool addNodeUpdate      (rtl::Reference<ElementUpdate> const & _aNode);
            bool addPropertyUpdate  (rtl::Reference<ElementUpdate> const & _aProp);
            void removeNodeByName      (rtl::OUString const & _aName);
            void removePropertyByName  (rtl::OUString const & _aName);

            Op getOperation() const { return m_op; }

            rtl::Reference<ElementUpdate> getNodeByName      (rtl::OUString const & _aName) const;
            rtl::Reference<ElementUpdate> getPropertyByName  (rtl::OUString const & _aName) const;

            ElementList::const_iterator beginNodes()       const { return m_aNodes.begin(); }
            ElementList::const_iterator endNodes()         const { return m_aNodes.end();   };
            ElementList::const_iterator beginProperties()  const { return m_aProperties.begin(); };
            ElementList::const_iterator endProperties()    const { return m_aProperties.end();   };

            bool hasChildren() const { return !m_aNodes.empty() || !m_aProperties.empty(); }

            void writeChildrenToLayer(backenduno::XLayerHandler * _pLayer);
        private:
            ElementList m_aNodes;
            ElementList m_aProperties;
            ElementList m_aRemovedElements;
            Op          m_op;
        };
// -----------------------------------------------------------------------------

        class NodeModification : public NodeUpdate
        {
        public:
            NodeModification(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset);
            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class NodeReplace : public NodeUpdate
        {
        public:
            NodeReplace(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags);
            NodeReplace(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, rtl::OUString const & _aTemplateName, rtl::OUString const & _aTemplateComponent);

            bool hasTemplate() const;
            rtl::OUString getTemplateName()      const { return m_aTemplateName; }
            rtl::OUString getTemplateComponent() const { return m_aTemplateComponent; }

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        private:
            rtl::OUString m_aTemplateName;
            rtl::OUString m_aTemplateComponent;
        };
// -----------------------------------------------------------------------------

        class NodeDrop : public ElementUpdate
        {
        public:
            NodeDrop(NodeUpdate * _pParent, rtl::OUString const & _aName);

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class PropertyUpdate : public ElementUpdate
        {
            typedef std::map< rtl::OUString, uno::Any > ValueList;

            ValueList m_aValues;
            uno::Type m_aType;
        public:
            PropertyUpdate(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType);

            bool setValueFor(rtl::OUString const & _aLocale, uno::Any const & _aValueUpdate);
            bool resetValueFor(rtl::OUString const & _aLocale);
            void removeValueFor(rtl::OUString const & _aLocale);

            bool setValue(uno::Any const & _aValueUpdate)    { return setValueFor(primarySlot(), _aValueUpdate); }
            bool resetValue()                                   { return resetValueFor(primarySlot()); }
            void removeValue()                                  { removeValueFor(primarySlot()); }

            void finishValue();

            uno::Type const & getValueType()    const { return m_aType; }

            bool hasValueFor(rtl::OUString const & _aLocale) const;
            bool hasValue() const { return hasValueFor(primarySlot()); }

            bool hasResetFor(rtl::OUString const & _aLocale) const;
            bool hasReset() const { return hasResetFor(primarySlot()); }

            bool hasChangeFor(rtl::OUString const & _aLocale) const;
            bool hasChange() const { return hasChangeFor(primarySlot()); }

            uno::Any getValueFor(rtl::OUString const & _aLocale) const;
            uno::Any getValue() const { return getValueFor(primarySlot()); }

            ValueList::const_iterator beginValues()  const { return m_aValues.begin(); }
            ValueList::const_iterator endValues()    const { return m_aValues.end(); }

            void writeValueToLayerFor(backenduno::XLayerHandler * _pLayer, uno::Any const & _aValue, rtl::OUString const & _aLocale);
            void writeValueToLayer(backenduno::XLayerHandler * _pLayer, uno::Any const & _aValue);
            void writeValuesToLayer(backenduno::XLayerHandler * _pLayer);
            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        private:
            rtl::OUString primarySlot() const { return rtl::OUString(); }

            static uno::Any const & getResetMarker();
            static inline bool isResetMarker(uno::Any const & _aValue);

            virtual PropertyUpdate  * asPropertyUpdate();
        };
// -----------------------------------------------------------------------------

        class PropertyAdd : public ElementUpdate
        {
            uno::Type m_aValueType;
            uno::Any  m_aValue;
        public:
            PropertyAdd(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType);
            PropertyAdd(NodeUpdate * _pParent, rtl::OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue);

            bool hasValue() const { return !! m_aValue.hasValue(); }
            uno::Any  const & getValue()        const { return m_aValue; }
            uno::Type const & getValueType()    const { return m_aValueType; }

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class PropertyReset : public ElementUpdate
        {
        public:
            PropertyReset(NodeUpdate * _pParent, rtl::OUString const & _aName);

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




