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

#include <vcl/window.hxx>

#include "invmerge.hxx"



ScInvertMerger::ScInvertMerger( ::std::vector< Rectangle >* pRectangles ) :
    pRects( pRectangles )
{
    
}

ScInvertMerger::~ScInvertMerger()
{
    Flush();
}

void ScInvertMerger::Flush()
{
    FlushLine();
    FlushTotal();

    OSL_ENSURE( aLineRect.IsEmpty() && aTotalRect.IsEmpty(), "Flush: not empty" );

    if ( pRects )
    {
        //
        
        //

        size_t nComparePos = 0;
        while ( nComparePos < pRects->size() )
        {
            Rectangle aCompRect = (*pRects)[nComparePos];
            sal_Int32 nBottom = aCompRect.Bottom();
            size_t nOtherPos = nComparePos + 1;

            while ( nOtherPos < pRects->size() )
            {
                Rectangle aOtherRect = (*pRects)[nOtherPos];
                if ( aOtherRect.Top() > nBottom + 1 )
                {
                    
                    break;
                }
                if ( aOtherRect.Top() == nBottom + 1 &&
                     aOtherRect.Left() == aCompRect.Left() &&
                     aOtherRect.Right() == aCompRect.Right() )
                {
                    
                    nBottom = aOtherRect.Bottom();
                    aCompRect.Bottom() = nBottom;
                    (*pRects)[nComparePos].Bottom() = nBottom;

                    
                    pRects->erase( pRects->begin() + nOtherPos );

                    
                }
                else
                    ++nOtherPos;
            }

            ++nComparePos;
        }
    }
}

void ScInvertMerger::FlushTotal()
{
    if( aTotalRect.IsEmpty() )
        return;                         

    if ( pRects )
        pRects->push_back( aTotalRect );

    aTotalRect.SetEmpty();
}

void ScInvertMerger::FlushLine()
{
    if( aLineRect.IsEmpty() )
        return;                         

    if ( aTotalRect.IsEmpty() )
    {
        aTotalRect = aLineRect;         
    }
    else
    {
        if ( aLineRect.Left()  == aTotalRect.Left()  &&
             aLineRect.Right() == aTotalRect.Right() &&
             aLineRect.Top()   == aTotalRect.Bottom() + 1 )
        {
            
            aTotalRect.Bottom() = aLineRect.Bottom();
        }
        else
        {
            FlushTotal();                   
            aTotalRect = aLineRect;         
        }
    }

    aLineRect.SetEmpty();
}

void ScInvertMerger::AddRect( const Rectangle& rRect )
{
    Rectangle aJustified = rRect;
    if ( rRect.Left() > rRect.Right() )     
    {
        aJustified.Left() = rRect.Right();
        aJustified.Right() = rRect.Left();
    }

    if ( aLineRect.IsEmpty() )
    {
        aLineRect = aJustified;             
    }
    else
    {
        sal_Bool bDone = false;
        if ( aJustified.Top()    == aLineRect.Top()    &&
             aJustified.Bottom() == aLineRect.Bottom() )
        {
            
            if ( aJustified.Left() == aLineRect.Right() + 1 )
            {
                aLineRect.Right() = aJustified.Right();
                bDone = sal_True;
            }
            else if ( aJustified.Right() + 1 == aLineRect.Left() )  
            {
                aLineRect.Left() = aJustified.Left();
                bDone = sal_True;
            }
        }
        if (!bDone)
        {
            FlushLine();                
            aLineRect = aJustified;     
        }
    }
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
