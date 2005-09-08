/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerupdatebuilder.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:31:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEBUILDER_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATEBUILDER_HXX

#ifndef CONFIGMGR_BACKEND_LAYERUPDATE_HXX
#include "layerupdate.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace com { namespace sun { namespace star { namespace configuration { namespace backend {
    struct TemplateIdentifier;
} } } } }

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
        class NodeUpdate;
        class PropertyUpdate;

        class LayerUpdateBuilder
        {
            LayerUpdate     m_aUpdate;
            NodeUpdate      * m_pCurrentNode;
            PropertyUpdate  * m_pCurrentProp;
        public:
            LayerUpdateBuilder();

        public:
            /// initialize the update
            bool init();

            bool modifyNode(OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, sal_Bool _bReset);
            bool replaceNode(OUString const & _aName, sal_Int16 _nFlags, backenduno::TemplateIdentifier const * _pTemplate = NULL);
            bool finishNode();

            bool removeNode(OUString const & _aName);

            bool modifyProperty(OUString const & _aName, sal_Int16 _nFlags, sal_Int16 _nFlagsMask, uno::Type const & _aType);
            bool setPropertyValue(uno::Any const & _aValue);
            bool setPropertyValueForLocale(uno::Any const & _aValue, OUString const & _aLocale);
            bool resetPropertyValue();
            bool resetPropertyValueForLocale(OUString const & _aLocale);
            bool finishProperty();

            bool addNullProperty(OUString const & _aName, sal_Int16 _nFlags, uno::Type const & _aType);
            bool addProperty(OUString const & _aName, sal_Int16 _nFlags, uno::Any const & _aValue);
            bool resetProperty(OUString const & _aName);

            bool finish();
            void clear();

            bool isEmpty()      const;
            bool isActive()     const;
            bool isComplete()   const;

            bool isPropertyActive()     const;

            LayerUpdate const & result() const;
        private:
            LayerUpdate & data();
        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




