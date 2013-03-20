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

#ifndef _ACCFRAME_HXX
#define _ACCFRAME_HXX

#include <swrect.hxx>

#include <sal/types.h>
#include <tools/string.hxx>

#include <list>
#include <accfrmobj.hxx>

class SwAccessibleMap;
class SwFrm;
class ViewShell;
namespace sw { namespace access {
    class SwAccessibleChild;
}}

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleFrame
{
    SwRect maVisArea;
    const SwFrm* mpFrm;
    const sal_Bool mbIsInPagePreview;

protected:
    // #i77106# - method needs to be called by new class <SwAccessibleTableColHeaders>
    static sal_Int32 GetChildCount( SwAccessibleMap& rAccMap,
                                    const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    sal_Bool bInPagePreview );
private:
    static sw::access::SwAccessibleChild GetChild( SwAccessibleMap& rAccMap,
                                                   const SwRect& rVisArea,
                                                   const SwFrm& rFrm,
                                                   sal_Int32& rPos,
                                                   sal_Bool bInPagePreview);

    static sal_Bool GetChildIndex( SwAccessibleMap& rAccMap,
                                   const SwRect& rVisArea,
                                   const SwFrm& rFrm,
                                   const sw::access::SwAccessibleChild& rChild,
                                   sal_Int32& rPos,
                                   sal_Bool bInPagePreview );

    static sw::access::SwAccessibleChild GetChildAtPixel( const SwRect& rVisArea,
                                                          const SwFrm& rFrm,
                                                          const Point& rPos,
                                                          sal_Bool bInPagePreview,
                                                          SwAccessibleMap& rAccMap );

    static void GetChildren( SwAccessibleMap& rAccMap,
                             const SwRect& rVisArea,
                             const SwFrm& rFrm,
                             ::std::list< sw::access::SwAccessibleChild >& rChildren,
                             sal_Bool bInPagePreview );

protected:
    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    sal_Bool IsShowing( const SwAccessibleMap& rAccMap,
                        const sw::access::SwAccessibleChild& rFrmOrObj ) const;
    inline sal_Bool IsShowing( const SwRect& rFrm ) const;
    inline sal_Bool IsShowing( const SwAccessibleMap& rAccMap ) const;

    inline sal_Bool IsInPagePreview() const
    {
        return mbIsInPagePreview;
    }

    inline void ClearFrm()
    {
        mpFrm = 0;
    }

    SwAccessibleFrame( const SwRect& rVisArea,
                       const SwFrm *pFrm,
                       sal_Bool bIsPagePreview );
    virtual ~SwAccessibleFrame();

public:
    // Return the SwFrm this context is attached to.
    const SwFrm* GetFrm() const { return mpFrm; };

    static const SwFrm* GetParent( const sw::access::SwAccessibleChild& rFrmOrObj,
                                   sal_Bool bInPagePreview );

    sal_Int32 GetChildIndex( SwAccessibleMap& rAccMap,
                             const sw::access::SwAccessibleChild& rChild ) const;

protected:
    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    SwRect GetBounds( const SwAccessibleMap& rAccMap,
                      const SwFrm *pFrm = 0 );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    inline const SwFrm *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    sal_Int32 GetChildCount( SwAccessibleMap& rAccMap ) const;
    sw::access::SwAccessibleChild GetChild( SwAccessibleMap& rAccMap,
                                            sal_Int32 nPos ) const;
    sw::access::SwAccessibleChild GetChildAtPixel( const Point& rPos,
                                                   SwAccessibleMap& rAccMap ) const;
    void GetChildren( SwAccessibleMap& rAccMap,
                      ::std::list< sw::access::SwAccessibleChild >& rChildren ) const;

    inline void SetVisArea( const SwRect& rNewVisArea )
    {
        maVisArea = rNewVisArea;
    }

    inline const SwRect& GetVisArea() const
    {
        return maVisArea;
    }

    String GetFormattedPageNumber() const;
};

inline sal_Bool SwAccessibleFrame::IsShowing( const SwRect& rFrm ) const
{
    return rFrm.IsOver( maVisArea );
}

inline sal_Bool SwAccessibleFrame::IsShowing( const SwAccessibleMap& rAccMap ) const
{
    sw::access::SwAccessibleChild aFrmOrObj( GetFrm() );
    return IsShowing( rAccMap, aFrmOrObj );
}

inline const SwFrm *SwAccessibleFrame::GetParent() const
{
    sw::access::SwAccessibleChild aFrmOrObj( GetFrm() );
    return GetParent( aFrmOrObj, IsInPagePreview()  );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
