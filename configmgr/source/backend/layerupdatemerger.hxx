/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layerupdatemerger.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:32:30 $
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

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEMERGER_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATEMERGER_HXX

#ifndef CONFIGMGR_BACKEND_BASICUPDATEMERGER_HXX
#include "basicupdatemerger.hxx"
#endif

#ifndef CONFIGMGR_BACKEND_LAYERUPDATE_HXX
#include "layerupdate.hxx"
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        using rtl::OUString;
// -----------------------------------------------------------------------------
        class NodeUpdate;
        class PropertyUpdate;
        typedef rtl::Reference<NodeUpdate>      NodeUpdateRef;
        typedef rtl::Reference<PropertyUpdate>  PropertyUpdateRef;

        class LayerUpdateMerger : protected BasicUpdateMerger
        {
        public:
            static LayerSource getMergedLayer(LayerSource const & _xSourceLayer, LayerUpdate const & _aLayerUpdate)
            { return new LayerUpdateMerger(_xSourceLayer, _aLayerUpdate); }

        public:
            explicit
            LayerUpdateMerger( LayerSource const & _xSourceLayer, LayerUpdate const & _aLayerUpdate);

            ~LayerUpdateMerger();


        // XLayerHandler overrides
        protected:
            virtual void SAL_CALL
                startLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        // BasicUpdateMerger
            virtual void flushUpdate();
        private:
            void malformedUpdate(sal_Char const * pMsg);
            void illegalUpdate(sal_Char const * pMsg);
        private:
            LayerUpdate                 m_aLayerUpdate;
            NodeUpdateRef               m_xCurrentNode;
            PropertyUpdateRef           m_xCurrentProp;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




