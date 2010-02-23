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
#ifndef _ACCFRAME_HXX
#define _ACCFRAME_HXX

#include <sal/types.h>

#include <list>
#include <accfrmobj.hxx>

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleMap;

class SwAccessibleFrame
{
    SwRect maVisArea;
    const SwFrm* mpFrm;
    const sal_Bool mbIsInPagePreview;

protected:
    // --> OD 2007-06-29 #i77106#
    // method needs to be called by new class <SwAccessibleTableColHeaders>
    static sal_Int32 GetChildCount( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    sal_Bool bInPagePreview );
private:
    static SwFrmOrObj GetChild( const SwRect& rVisArea,
                                  const SwFrm *pFrm,
                                  sal_Int32& rPos,
                                sal_Bool bInPagePreview);
    static sal_Bool GetChildIndex( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const SwFrmOrObj& rChild,
                                    sal_Int32& rPos,
                                    sal_Bool bInPagePreview );
    static SwFrmOrObj GetChildAtPixel( const SwRect& rVisArea,
                                    const SwFrm *pFrm,
                                    const Point& rPos,
                                    sal_Bool bInPagePreview,
                                    const SwAccessibleMap *pMap );
    static void GetChildren( const SwRect& rVisArea, const SwFrm *pFrm,
                             ::std::list< SwFrmOrObj >& rChildren,
                             sal_Bool bInPagePreview );

protected:

    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    inline sal_Bool IsShowing( const SwRect& rFrm ) const;
    inline sal_Bool IsShowing( const SwFrmOrObj& rFrmOrObj ) const;
    inline sal_Bool IsShowing() const;

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
    const SwFrm *GetFrm() const { return mpFrm; };


    static const SwFrm *GetParent( const SwFrmOrObj& rFrmOrObj,
                                   sal_Bool bInPagePreview );

protected:

    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    SwRect GetBounds( const SwFrm *pFrm=0 );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    inline const SwFrm *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    inline sal_Int32 GetChildCount() const;
    inline SwFrmOrObj GetChild( sal_Int32 nPos ) const;
    inline sal_Int32 GetChildIndex( const SwFrmOrObj& rChild ) const;
    inline SwFrmOrObj GetChildAtPixel( const Point& rPos,
                                    const SwAccessibleMap *pMap ) const;
    inline void GetChildren( ::std::list< SwFrmOrObj >& rChildren ) const;

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

inline sal_Bool SwAccessibleFrame::IsShowing( const SwFrmOrObj& rFrmOrObj ) const
{
    return IsShowing( rFrmOrObj.GetBox() );
}

inline sal_Bool SwAccessibleFrame::IsShowing() const
{
    SwFrmOrObj aFrmOrObj( GetFrm() );
    return IsShowing( aFrmOrObj );
}

inline const SwFrm *SwAccessibleFrame::GetParent() const
{
    SwFrmOrObj aFrmOrObj( GetFrm() );
    return GetParent( aFrmOrObj, IsInPagePreview()  );
}

inline sal_Int32 SwAccessibleFrame::GetChildCount() const
{
    return GetChildCount( maVisArea, mpFrm, IsInPagePreview() );
}

inline SwFrmOrObj SwAccessibleFrame::GetChild( sal_Int32 nPos ) const
{
    return GetChild( maVisArea, mpFrm, nPos, IsInPagePreview() );
}

inline sal_Int32 SwAccessibleFrame::GetChildIndex( const SwFrmOrObj& rChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( maVisArea, mpFrm, rChild, nPos, IsInPagePreview() ) ? nPos : -1L;
}

inline SwFrmOrObj SwAccessibleFrame::GetChildAtPixel( const Point& rPos,
                                    const SwAccessibleMap *pMap ) const
{
    return GetChildAtPixel( maVisArea, mpFrm, rPos, IsInPagePreview(), pMap );
}

inline void SwAccessibleFrame::GetChildren( ::std::list< SwFrmOrObj >& rChildren ) const
{
    GetChildren( maVisArea, mpFrm, rChildren, IsInPagePreview() );
}
#endif

