/*************************************************************************
 *
 *  $RCSfile: accframe.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $ $Date: 2002-02-20 17:55:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ACCFRAME_HXX
#define _ACCFRAME_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _SV_GEN_HXX
#include <vcl/gen.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif


class SwAccessibleFrame
{
    Rectangle aVisArea;
    const SwFrm *pFrm;

    static sal_Int32 GetChildCount( const Rectangle& rVisArea,
                                    const SwFrm *pFrm );
    static const SwFrm *GetChild( const Rectangle& rVisArea,
                                  const SwFrm *pFrm,
                                  sal_Int32& rPos );
    static sal_Bool GetChildIndex( const Rectangle& rVisArea,
                                        const SwFrm *pFrm,
                                        const SwFrm *pChild,
                                        sal_Int32& rPos );
    static const SwFrm *GetChildAt( const Rectangle& rVisArea,
                                    const SwFrm *pFrm,
                                    const Point& rPos );
protected:

    // A child has been added while setting the vis area
    virtual sal_Bool ChildScrolledIn( const SwFrm *pFrm );

    // A child has been removed while setting the vis area
    virtual sal_Bool ChildScrolledOut( const SwFrm *pFrm );

    // A child has been moved while setting the vis area
    virtual sal_Bool ChildScrolled( const SwFrm *pFrm );

    // A child shall be disposed
    virtual sal_Bool DisposeChild( const SwFrm *pFrm, sal_Bool bRecursive );

    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    inline sal_Bool IsShowing( const Rectangle& rFrm ) const;
    inline sal_Bool IsShowing( const SwFrm *pFrm ) const;
    inline sal_Bool IsShowing() const { return IsShowing( GetFrm() ); }

    void ClearFrm() { pFrm = 0; }

public:

    SwAccessibleFrame( const Rectangle& rVisArea,
                       const SwFrm *pFrm );
    virtual ~SwAccessibleFrame();

    // Does a frame of theis type may have an accessible?
    inline static sal_Bool IsAccessible( const SwFrm *pFrm );

    // Return the SwFrm this context is attached to.
    const SwFrm *GetFrm() const { return pFrm; };

    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    Rectangle GetBounds( const SwFrm *pFrm=0 );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    const SwFrm *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    inline sal_Int32 GetChildCount() const;
    inline const SwFrm *GetChild( sal_Int32 nPos ) const;
    inline sal_Int32 GetChildIndex( const SwFrm *pFrm ) const;
    inline const SwFrm *GetChildAt( const Point& rPos ) const;

    static void SetVisArea( const SwFrm *pFrm,
                            const Rectangle& rOldVisArea,
                            const Rectangle& rNewVisArea,
                            SwAccessibleFrame *pAcc = 0 );
    inline void SetVisArea( const Rectangle& rNewVisArea );

    static void DisposeChildren( const SwFrm *pFrm,
                                  const Rectangle& rOldVisArea,
                                 sal_Bool bRecursive,
                                  SwAccessibleFrame *pAcc = 0 );
    inline void DisposeChildren( sal_Bool bRecursive );

    const Rectangle& GetVisArea() const { return aVisArea; }

};

inline sal_Bool SwAccessibleFrame::IsShowing( const Rectangle& rFrm ) const
{
    return rFrm.IsOver( aVisArea );
}

inline sal_Bool SwAccessibleFrame::IsShowing( const SwFrm *pFrm ) const
{
    return IsShowing( pFrm->Frm().SVRect() );
}

inline sal_Int32 SwAccessibleFrame::GetChildCount() const
{
    return GetChildCount( aVisArea, pFrm );
}

inline const SwFrm *SwAccessibleFrame::GetChild( sal_Int32 nPos ) const
{
    return GetChild( aVisArea, pFrm, nPos );
}

inline sal_Int32 SwAccessibleFrame::GetChildIndex( const SwFrm *pChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( aVisArea, pFrm, pChild, nPos ) ? nPos : -1L;
}

inline const SwFrm *SwAccessibleFrame::GetChildAt( const Point& rPos ) const
{
    return GetChildAt( aVisArea, pFrm, rPos );
}

inline sal_Bool SwAccessibleFrame::IsAccessible( const SwFrm *pFrm )
{
    return pFrm->IsTxtFrm() || pFrm->IsRootFrm();
}

inline void SwAccessibleFrame::SetVisArea( const Rectangle& rNewVisArea )
{
    Rectangle aOldVisArea( aVisArea );
    aVisArea = rNewVisArea;

    SetVisArea( GetFrm(), aOldVisArea, aVisArea, this );
}

inline void SwAccessibleFrame::DisposeChildren( sal_Bool bRecursive )
{
    DisposeChildren( GetFrm(), aVisArea, bRecursive, this );
}

#endif

