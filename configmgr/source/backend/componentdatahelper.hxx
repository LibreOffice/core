/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componentdatahelper.hxx,v $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX
#define CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX

#include "valuenode.hxx"
#include "utility.hxx"
#include "stack.hxx"
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include "mergeddataprovider.hxx"
#ifndef CONFIGMGR_LOGGER_HXX_
#include "logger.hxx"
#endif
#include "request.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    class OTreeNodeFactory;
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno        = ::com::sun::star::uno;
        namespace backenduno = ::com::sun::star::configuration::backend;

        using  backenduno::TemplateIdentifier;

        using ::rtl::OUString;

// -----------------------------------------------------------------------------

        class DataBuilderContext
        {
            Logger                      m_aLogger;
            Stack< ISubtree * >         m_aParentStack;
            OUString                    m_aActiveComponent;
            uno::XInterface *           m_pContext;
            OUString                    m_aExpectedComponentName;
            ITemplateDataProvider *     m_aTemplateProvider;
        public:
            typedef uno::Reference< uno::XComponentContext > UnoContext;
            explicit DataBuilderContext(UnoContext const & xContext);
            DataBuilderContext(UnoContext const & xContext, uno::XInterface * _pContext , ITemplateDataProvider*  aTemplateProvider = NULL);
            DataBuilderContext(UnoContext const & xContext, uno::XInterface * _pContext, const OUString& aExpectedComponentName,ITemplateDataProvider*  aTemplateProvider = NULL );
            DataBuilderContext(DataBuilderContext const & aBaseContext, uno::XInterface * _pContext);
            ~DataBuilderContext();

            bool        isDone() const;

            bool        hasActiveComponent()    const { return m_aActiveComponent.getLength() != 0; }
            OUString    getActiveComponent()    const { return m_aActiveComponent; }

            ISubtree &          getCurrentParent()
                CFG_UNO_THROW1( configuration::backend::MalformedDataException )
                { return implGetCurrentParent(); }

            ISubtree const &    getCurrentParent() const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException )
                { return implGetCurrentParent(); }

            node::Attributes    getCurrentAttributes() const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException )
                { return implGetCurrentParent().getAttributes(); }

            ITemplateDataProvider * getTemplateProvider() const
            { return  m_aTemplateProvider; }

            OUString getTemplateComponent(TemplateIdentifier const & aItemType ) const;

            TemplateIdentifier stripComponent   (TemplateIdentifier const & aItemType ) const;
            TemplateIdentifier completeComponent(TemplateIdentifier const & aItemType ) const;

            TemplateIdentifier getCurrentItemType() const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            TemplateIdentifier getValidItemType(TemplateIdentifier const & aItemType) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void startActiveComponent(OUString const & _aComponent)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void endActiveComponent()
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            bool isProperty(INode * pProp) const
                CFG_UNO_THROW_RTE();

            bool isNode(INode * pNode) const CFG_UNO_THROW_RTE()
            { return !isProperty(pNode); }

            void pushNode(ISubtree * pTree)
                CFG_UNO_THROW_RTE();
            void popNode()
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            INode    * findProperty(OUString const & _aName)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            ISubtree * findNode(OUString const & _aName)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            bool isWritable(INode const * pNode) const
                CFG_NOTHROW(  );
            bool isRemovable(ISubtree const * pItem) const
                CFG_NOTHROW(  );

            ISubtree  * addNodeToCurrent(std::auto_ptr<ISubtree>  _aNode)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            ISubtree  * addLocalizedToCurrent(std::auto_ptr<ISubtree>  _aNode)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            ValueNode * addPropertyToCurrent(std::auto_ptr<ValueNode> _aNode, bool _bMayReplace = false)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void markCurrentMerged();

            // Logging support
            Logger const & getLogger() const { return m_aLogger; }

            OUString getNodeParentagePath() const;
            OUString getNodePath(OUString const & aNodeName) const;

            // Exception support
            void raiseMalformedDataException    (sal_Char const * _pText) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseNoSupportException        (sal_Char const * _pText) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseIllegalAccessException    (sal_Char const * _pText) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseIllegalArgumentException  (sal_Char const * _pText, sal_Int16 _nPos = 0) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseElementExistException     (sal_Char const * _pText, OUString const & _sElement) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseNoSuchElementException    (sal_Char const * _pText, OUString const & _sElement) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseUnknownPropertyException  (sal_Char const * _pText, OUString const & _sElement) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raisePropertyExistException    (sal_Char const * _pText, OUString const & _sElement) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            void raiseIllegalTypeException  (sal_Char const * _pText) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
            TemplateResult getTemplateData (TemplateRequest const & _aRequest  );
        private:
            INode * findChild(OUString const & _aName)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            OUString makeMessageWithPath(sal_Char const * _pText) const
                CFG_UNO_THROW_RTE(  );

            OUString makeMessageWithName(sal_Char const * _pText, OUString const & _aName) const
                CFG_UNO_THROW_RTE(  );

            ISubtree & implGetCurrentParent() const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

        };
// -----------------------------------------------------------------------------

        class ComponentDataFactory
        {
            OTreeNodeFactory &          m_rNodeFactory;
        public:
            ComponentDataFactory();

            ComponentDataFactory(OTreeNodeFactory & _rNodeFactory)
            : m_rNodeFactory(_rNodeFactory)
            {}

        public:
            OTreeNodeFactory&   getNodeFactory() const { return m_rNodeFactory; }

            std::auto_ptr<ISubtree> createGroup(OUString const & _aName,
                                                bool _bExtensible,
                                                node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createSet( OUString const & _aName,
                                                TemplateIdentifier const & aItemType,
                                                bool _bExtensible,
                                                node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createLocalizedContainer(OUString const & _aName,
                                                             uno::Type const & _aValueType,
                                                             node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createPlaceHolder(OUString const & _aName,
                                                        TemplateIdentifier const & _aInstanceType) const;

            static bool isInstancePlaceHolder(ISubtree const & _aInstanceTree);
            static TemplateIdentifier getInstanceType(ISubtree const & _aInstanceTree);
        };
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




