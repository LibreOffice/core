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

#include "file/quotedstring.hxx"
#include <rtl/ustrbuf.hxx>

namespace connectivity
{
    
    
    
    
    sal_Int32 QuotedTokenizedString::GetTokenCount( sal_Unicode cTok, sal_Unicode cStrDel ) const
    {
        SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com QuotedTokenizedString::GetTokenCount" );
        const sal_Int32 nLen = m_sString.getLength();
        if ( !nLen )
            return 0;

        sal_Int32 nTokCount = 1;
        bool bStart = true;     
        bool bInString = false; 

        
        for( sal_Int32 i = 0; i < nLen; ++i )
        {
            const sal_Unicode cChar = m_sString[i];
            if (bStart)
            {
                bStart = false;
                
                if ( cChar == cStrDel )
                {
                    bInString = true;   
                    continue;           
                }
            }

            if (bInString)
            {
                
                if ( cChar == cStrDel )
                {
                    if ((i+1 < nLen) && (m_sString[i+1] == cStrDel))
                    {
                        
                        ++i;    
                    }
                    else
                    {
                        
                        bInString = false;
                    }
                }
            } 
            else
            {
                
                if ( cChar == cTok )
                {
                    ++nTokCount;
                    bStart = true;
                }
            }
        }
        

        return nTokCount;
    }

    
    OUString QuotedTokenizedString::GetTokenSpecial(sal_Int32& nStartPos, sal_Unicode cTok, sal_Unicode cStrDel) const
    {
        SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com QuotedTokenizedString::GetTokenCount" );
        const sal_Int32 nLen = m_sString.getLength();
        if ( nLen )
        {
            bool bInString = (nStartPos < nLen) && (m_sString[nStartPos] == cStrDel);   

            
            if (bInString )
                ++nStartPos;            
            if ( nStartPos >= nLen )
                return OUString();

            OUStringBuffer sBuff( nLen - nStartPos + 1 );

            
            for( sal_Int32 i = nStartPos; i < nLen; ++i )
            {
                const sal_Unicode cChar = m_sString[i];
                if (bInString)
                {
                    
                    if ( cChar == cStrDel )
                    {
                        if ((i+1 < nLen) && (m_sString[i+1] == cStrDel))
                        {
                            
                            
                            ++i;
                            sBuff.append(m_sString[i]);    
                        }
                        else
                        {
                            
                            bInString = false;
                        }
                    }
                    else
                    {
                        sBuff.append(cChar);
                    }
                }
                else
                {
                    
                    if ( cChar == cTok )
                    {
                        
                        nStartPos = i+1;
                        break;
                    }
                    else
                    {
                        sBuff.append(cChar);
                    }
                }
            } 
            return sBuff.makeStringAndClear();
        }
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
