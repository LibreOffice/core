/*************************************************************************
 *
 *  $RCSfile: accframe.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: mib $ $Date: 2002-04-05 12:04:45 $
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
#ifndef _ACCFRMOBJ_HXX
#include <accfrmobj.hxx>
#endif

// Any method of this class must be called with an acquired solar mutex!

class SwAccessibleFrame
{
    Rectangle aVisArea;
    const SwFrm *pFrm;

    static sal_Int32 GetChildCount( const Rectangle& rVisArea,
                                    const SwFrm *pFrm );
    static SwFrmOrObj GetChild( const Rectangle& rVisArea,
                                  const SwFrm *pFrm,
                                  sal_Int32& rPos );
    static sal_Bool GetChildIndex( const Rectangle& rVisArea,
                                    const SwFrm *pFrm,
                                    const SwFrmOrObj& rChild,
                                    sal_Int32& rPos );
    static SwFrmOrObj GetChildAt( const Rectangle& rVisArea,
                                    const SwFrm *pFrm,
                                    const Point& rPos );

    static void MergeLowerBounds( SwRect& rBounds,
                                  const Rectangle& rVisArea,
                                  const SwFrm *pFrm );
protected:

    // A child has been added while setting the vis area
    virtual sal_Bool ChildScrolledIn( const SwFrm *pFrm );

    // A child has been removed while setting the vis area
    virtual sal_Bool ChildScrolledOut( const SwFrm *pFrm );

    // A child has been moved while setting the vis area
    virtual sal_Bool ChildScrolled( const SwFrm *pFrm );

    // The editable state of a child should be checked
    virtual sal_Bool CheckStatesChild( const SwFrm *pFrm, sal_uInt8 nStates );

    // A child shall be disposed
    virtual sal_Bool DisposeChild( const SwFrm *pFrm, sal_Bool bRecursive );

    sal_Bool IsEditable( ViewShell *pVSh ) const;

    sal_Bool IsOpaque( ViewShell *pVSh ) const;

    inline sal_Bool IsShowing( const Rectangle& rFrm ) const;
    inline sal_Bool IsShowing( const SwFrm *pFrm ) const;
    inline sal_Bool IsShowing() const { return IsShowing( GetFrm() ); }

    void ClearFrm() { pFrm = 0; }

    SwAccessibleFrame( const Rectangle& rVisArea,
                       const SwFrm *pFrm );
    virtual ~SwAccessibleFrame();

public:
    // Return the SwFrm this context is attached to.
    const SwFrm *GetFrm() const { return pFrm; };

    static const SwFrm *GetParent( const SwFrm *pFrm );

protected:

    // Return the bounding box of the frame clipped to the vis area. If
    // no frame is specified, use this' frame.
    Rectangle GetBounds( const SwFrm *pFrm=0 );

    // Return the upper that has a context attached. This might be
    // another one than the immediate upper.
    inline const SwFrm *GetParent() const;

    // Return the lower count or the nth lower, there the lowers have a
    // not be same one as the SwFrm's lowers
    inline sal_Int32 GetChildCount() const;
    inline SwFrmOrObj GetChild( sal_Int32 nPos ) const;
    inline sal_Int32 GetChildIndex( const SwFrmOrObj& rChild ) const;
    inline SwFrmOrObj GetChildAt( const Point& rPos ) const;

    static void SetVisArea( const SwFrm *pFrm,
                            const Rectangle& rOldVisArea,
                            const Rectangle& rNewVisArea,
                            SwAccessibleFrame *pAcc = 0 );
    virtual void SetVisArea( const Rectangle& rNewVisArea );

    static void CheckStatesChildren( const SwFrm *pFrm,
                                  const Rectangle& rOldVisArea,
                                 sal_uInt8 nStates,
                                  SwAccessibleFrame *pAcc = 0 );
    inline void CheckStatesChildren( sal_uInt8 nStates );
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

inline const SwFrm *SwAccessibleFrame::GetParent() const
{
    return GetParent( pFrm );
}

inline sal_Int32 SwAccessibleFrame::GetChildCount() const
{
    return GetChildCount( aVisArea, pFrm );
}

inline SwFrmOrObj SwAccessibleFrame::GetChild( sal_Int32 nPos ) const
{
    return GetChild( aVisArea, pFrm, nPos );
}

inline sal_Int32 SwAccessibleFrame::GetChildIndex( const SwFrmOrObj& rChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( aVisArea, pFrm, rChild, nPos ) ? nPos : -1L;
}

inline SwFrmOrObj SwAccessibleFrame::GetChildAt( const Point& rPos ) const
{
    return GetChildAt( aVisArea, pFrm, rPos );
}

inline void SwAccessibleFrame::CheckStatesChildren( sal_uInt8 nStates )
{
    CheckStatesChildren( GetFrm(), aVisArea, nStates, this );
}

inline void SwAccessibleFrame::DisposeChildren( sal_Bool bRecursive )
{
    DisposeChildren( GetFrm(), aVisArea, bRecursive, this );
}

#endif

