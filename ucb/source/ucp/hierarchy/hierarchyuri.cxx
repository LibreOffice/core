/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"
#include "osl/diagnose.h"

#include "hierarchyuri.hxx"

using namespace hierarchy_ucp;



#define DEFAULT_DATA_SOURCE_SERVICE \
                    "com.sun.star.ucb.DefaultHierarchyDataSource"



//

//



void HierarchyUri::init() const
{
    
    if ( !m_aUri.isEmpty() && m_aPath.isEmpty() )
    {
        
        m_aService = m_aParentUri = m_aName = "";

        
        if ( ( m_aUri.getLength() < HIERARCHY_URL_SCHEME_LENGTH + 1 ) )
        {
            
            m_aPath = "/";
            return;
        }

        
        OUString aScheme
            = m_aUri.copy( 0, HIERARCHY_URL_SCHEME_LENGTH ).toAsciiLowerCase();
        if ( aScheme == HIERARCHY_URL_SCHEME )
        {
            m_aUri = m_aUri.replaceAt( 0, aScheme.getLength(), aScheme );

            sal_Int32 nPos = 0;

            
            

            if ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 1 )
            {
                
                m_aUri += "
                m_aService = DEFAULT_DATA_SOURCE_SERVICE ;

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() == HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri[ HIERARCHY_URL_SCHEME_LENGTH + 1 ] == '/' ) )
            {
                
                m_aUri += "/" DEFAULT_DATA_SOURCE_SERVICE "/";
                m_aService = DEFAULT_DATA_SOURCE_SERVICE;

                nPos = m_aUri.getLength() - 1;
            }
            else if ( ( m_aUri.getLength() > HIERARCHY_URL_SCHEME_LENGTH + 2 )
                      &&
                      ( m_aUri[ HIERARCHY_URL_SCHEME_LENGTH + 2 ] != '/' ) )
            {
                
                m_aUri = m_aUri.replaceAt(
                            HIERARCHY_URL_SCHEME_LENGTH + 2,
                            0,
                            OUString( "/" DEFAULT_DATA_SOURCE_SERVICE "/"  ) );
                m_aService = DEFAULT_DATA_SOURCE_SERVICE;

                nPos
                    = HIERARCHY_URL_SCHEME_LENGTH + 3 + m_aService.getLength();
            }
            else
            {
                
                sal_Int32 nStart = HIERARCHY_URL_SCHEME_LENGTH + 3;

                
                

                
                if ( nStart == m_aUri.getLength() )
                {
                    
                    m_aPath = "/";
                    return;
                }

                
                if ( m_aUri.indexOf(
                        OUString("
                        nStart ) != -1 )
                {
                    
                    m_aPath = "/";
                    return;
                }

                sal_Int32 nEnd = m_aUri.indexOf( '/', nStart );

                
                if ( nEnd == nStart )
                {
                    
                    m_aPath = "/";
                    return;
                }

                if ( nEnd == -1 )
                {
                    
                    nEnd = m_aUri.getLength();
                    m_aUri += "/";
                }

                m_aService = m_aUri.copy( nStart, nEnd - nStart );

                nPos = nEnd;
            }

            
            
            
            

            
            sal_Int32 nEnd = m_aUri.lastIndexOf( '/' );
            if ( ( nEnd > nPos ) && ( nEnd == ( m_aUri.getLength() - 1 ) ) )
                m_aUri = m_aUri.copy( 0, nEnd );

            
            m_aPath = m_aUri.copy( nPos );

            
            sal_Int32 nLastSlash = m_aUri.lastIndexOf( '/' );
            if ( ( nLastSlash != -1 ) &&
                 ( nLastSlash != m_aUri.getLength() - 1 ) ) 
            {
                m_aParentUri = m_aUri.copy( 0, nLastSlash );
                m_aName      = m_aUri.copy( nLastSlash + 1 );
            }

            
            m_bValid = true;
        }
        else
        {
            
            m_aPath = "/";
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
