/*************************************************************************
 *
 *  $RCSfile: updatedata.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:18:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_UPDATEDATA_HXX
#define CONFIGMGR_BACKEND_UPDATEDATA_HXX

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif

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
        using rtl::OUString;
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

        typedef rtl::Reference<ElementUpdate>   ElementUpdateRef;

// -----------------------------------------------------------------------------

        class NamedUpdate : public salhelper::SimpleReferenceObject
        {
            OUString const  m_aName;

        protected:
            explicit
            NamedUpdate(OUString const & _aName)
            : m_aName(_aName)
            {}

            ~NamedUpdate() {};

        public:
            OUString const & getName() const { return m_aName; }
        };
// -----------------------------------------------------------------------------
        class ElementUpdate : public NamedUpdate
        {
            NodeUpdate *    m_pParent;
            sal_Int16       m_nFlags;
            sal_Int16       m_nFlagsMask;
        protected:
            ElementUpdate(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask);

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
            typedef std::map< OUString, ElementUpdateRef > ElementList;
        public:
            enum Op { modify, reset, replace };

        protected:
            NodeUpdate(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, Op _op);

            virtual NodeUpdate * asNodeUpdate(bool _bMerged);

        public:
            bool addNodeUpdate      (ElementUpdateRef const & _aNode);
            bool addPropertyUpdate  (ElementUpdateRef const & _aProp);
            void removeNodeByName      (OUString const & _aName);
            void removePropertyByName  (OUString const & _aName);
            void clear(bool _bKeep);

            Op getOperation() const { return m_op; }

            ElementUpdateRef getNodeByName      (OUString const & _aName) const;
            ElementUpdateRef getPropertyByName  (OUString const & _aName) const;

            typedef ElementList::const_iterator Iterator;
            Iterator beginNodes()       const { return m_aNodes.begin(); }
            Iterator endNodes()         const { return m_aNodes.end();   };
            Iterator beginProperties()  const { return m_aProperties.begin(); };
            Iterator endProperties()    const { return m_aProperties.end();   };

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
            NodeModification(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset);
            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class NodeReplace : public NodeUpdate
        {
        public:
            NodeReplace(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags);
            NodeReplace(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, OUString const & _aTemplateName, OUString const & _aTemplateComponent);

            bool hasTemplate() const;
            OUString getTemplateName()      const { return m_aTemplateName; }
            OUString getTemplateComponent() const { return m_aTemplateComponent; }

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        private:
            OUString m_aTemplateName;
            OUString m_aTemplateComponent;
        };
// -----------------------------------------------------------------------------

        class NodeDrop : public ElementUpdate
        {
        public:
            NodeDrop(NodeUpdate * _pParent, OUString const & _aName);

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class PropertyUpdate : public ElementUpdate
        {
            typedef uno::Any ValueUpdate;
            typedef std::map< OUString, ValueUpdate > ValueList;

            ValueList m_aValues;
            uno::Type m_aType;
        public:
            PropertyUpdate(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType);

            bool setValueFor(OUString const & _aLocale, ValueUpdate const & _aValueUpdate);
            bool resetValueFor(OUString const & _aLocale);
            void removeValueFor(OUString const & _aLocale);

            bool setValue(ValueUpdate const & _aValueUpdate)    { return setValueFor(primarySlot(), _aValueUpdate); }
            bool resetValue()                                   { return resetValueFor(primarySlot()); }
            void removeValue()                                  { removeValueFor(primarySlot()); }

            void clear();

            uno::Type const & getValueType()    const { return m_aType; }

            bool hasValueFor(OUString const & _aLocale) const;
            bool hasValue() const { return hasValueFor(primarySlot()); }

            bool hasResetFor(OUString const & _aLocale) const;
            bool hasReset() const { return hasResetFor(primarySlot()); }

            bool hasChangeFor(OUString const & _aLocale) const;
            bool hasChange() const { return hasChangeFor(primarySlot()); }

            ValueUpdate getValueFor(OUString const & _aLocale) const;
            ValueUpdate getValue() const { return getValueFor(primarySlot()); }

            typedef ValueList::const_iterator Iterator;
            Iterator beginValues()  const { return m_aValues.begin(); }
            Iterator endValues()    const { return m_aValues.end(); }

            void writeValueToLayerFor(backenduno::XLayerHandler * _pLayer, ValueUpdate const & _aValue, OUString const & _aLocale);
            void writeValueToLayer(backenduno::XLayerHandler * _pLayer, ValueUpdate const & _aValue);
            void writeValuesToLayer(backenduno::XLayerHandler * _pLayer);
            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        private:
            OUString primarySlot() const { return OUString(); }

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
            PropertyAdd(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType);
            PropertyAdd(NodeUpdate * _pParent, OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue);

            bool hasValue() const { return !! m_aValue.hasValue(); }
            uno::Any  const & getValue()        const { return m_aValue; }
            uno::Type const & getValueType()    const { return m_aValueType; }

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

        class PropertyReset : public ElementUpdate
        {
        public:
            PropertyReset(NodeUpdate * _pParent, OUString const & _aName);

            virtual void writeToLayer(backenduno::XLayerHandler * _pLayer);
        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




