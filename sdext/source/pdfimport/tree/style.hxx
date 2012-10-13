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
