/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_PDFI_STYLE_HXX
#define INCLUDED_PDFI_STYLE_HXX

#include "pdfihelper.hxx"
#include <boost/unordered_map.hpp>
#include <vector>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include "treevisiting.hxx"

namespace pdfi
{
    struct Element;
    struct EmitContext;
    struct ElementTreeVisitable;

    class StyleContainer
    {
    public:
        struct Style
        {
            rtl::OString             Name;
            PropertyMap              Properties;
            rtl::OUString            Contents;
            Element*                 ContainedElement;
            std::vector< Style* >    SubStyles;

            Style() : ContainedElement( NULL )  {}
            Style( const rtl::OString& rName, const PropertyMap& rProps ) :
                Name( rName ),
                Properties( rProps ),
                ContainedElement( NULL )
            {}
        };

    private:
        struct HashedStyle
        {
            rtl::OString            Name;
            PropertyMap             Properties;
            rtl::OUString           Contents;
            Element*                ContainedElement;
            std::vector<sal_Int32>  SubStyles;

            bool                    IsSubStyle;
            sal_Int32               RefCount;

            HashedStyle() : ContainedElement( NULL ), IsSubStyle( true ), RefCount( 0 ) {}

            HashedStyle( const HashedStyle& rRight ) :
                Name( rRight.Name ),
                Properties( rRight.Properties ),
                Contents( rRight.Contents ),
                ContainedElement( rRight.ContainedElement ),
                SubStyles( rRight.SubStyles ),
                IsSubStyle( rRight.IsSubStyle ),
                RefCount( 0 )
            {}

            size_t hashCode() const
            {
                size_t nRet = size_t(Name.hashCode());
                for( PropertyMap::const_iterator it = Properties.begin();
                     it != Properties.end(); ++it )
                {
                     nRet ^= size_t(it->first.hashCode());
                     nRet ^= size_t(it->second.hashCode());
                }
                nRet = size_t(Contents.hashCode());
                nRet ^= size_t(ContainedElement);
                for( unsigned int n = 0; n < SubStyles.size(); ++n )
                     nRet ^= size_t(SubStyles[n]);
                return nRet;
            }

            bool operator==(const HashedStyle& rRight) const
            {
                if( Name != rRight.Name                 ||
                    Properties != rRight.Properties     ||
                    Contents != rRight.Contents         ||
                    ContainedElement != rRight.ContainedElement ||
                    SubStyles.size() != rRight.SubStyles.size()
                    )
                    return false;
                for( unsigned int n = 0; n < SubStyles.size(); ++n )
                {
                    if( SubStyles[n] != rRight.SubStyles[n] )
                        return false;
                }
                return true;
            }
        };

        struct StyleHash;
        friend struct StyleHash;
        struct StyleHash
        {
            size_t operator()( const StyleContainer::HashedStyle& rStyle ) const
            {
                return rStyle.hashCode();
            }
        };

        struct StyleIdNameSort;
        friend struct StyleIdNameSort;
        struct StyleIdNameSort
        {
            const boost::unordered_map< sal_Int32, HashedStyle >* m_pMap;

            StyleIdNameSort( const boost::unordered_map< sal_Int32, HashedStyle >* pMap ) :
                m_pMap(pMap)
            {}
            bool operator()( sal_Int32 nLeft, sal_Int32 nRight )
            {
                const boost::unordered_map< sal_Int32, HashedStyle >::const_iterator left_it =
                    m_pMap->find( nLeft );
                const boost::unordered_map< sal_Int32, HashedStyle >::const_iterator right_it =
                    m_pMap->find( nRight );
                if( left_it == m_pMap->end() )
                    return false;
                else if( right_it == m_pMap->end() )
                    return true;
                else
                    return left_it->second.Name < right_it->second.Name;
            }
        };

        sal_Int32                                               m_nNextId;
        boost::unordered_map< sal_Int32, HashedStyle >                 m_aIdToStyle;
        boost::unordered_map< HashedStyle, sal_Int32, StyleHash >      m_aStyleToId;

        void impl_emitStyle( sal_Int32           nStyleId,
                             EmitContext&        rContext,
                             ElementTreeVisitor& rContainedElemVisitor );

    public:
        StyleContainer();

        void emit( EmitContext&        rContext,
                   ElementTreeVisitor& rContainedElemVisitor );

        sal_Int32 impl_getStyleId( const Style& rStyle, bool bSubStyle );
        sal_Int32 getStyleId( const Style& rStyle )
        { return impl_getStyleId( rStyle, false ); }
        sal_Int32 getStandardStyleId( const rtl::OString& rFamily );

        // returns NULL for an invalid style id
        const PropertyMap* getProperties( sal_Int32 nStyleId ) const;
        sal_Int32 setProperties( sal_Int32 nStyleId, const PropertyMap &rNewProps );
        rtl::OUString getStyleName( sal_Int32 nStyle ) const;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
