/*************************************************************************
 *
 *  $RCSfile: opengrf.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2001-08-17 09:27:29 $
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
#ifndef _SVX_OPENGRF_HXX
#define _SVX_OPENGRF_HXX

#include <memory>       // auto_ptr

#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif

struct  SvxOpenGrf_Impl;

/**
   The class SvxOpenGraphicDialog encapsulates functionality to display
   an "insert graphics" dialog, as it is used e.g. in Draw or Writer.
   The dialog has a preview graphic and a "link" checkbox, by which the user
   can decide whether the document should only store a link to the selected graphic.
   This class is intended as a bare-bones replacement for the old SvxInsertGraphicDialog,
   with basically the same functionality.
 */

class SvxOpenGraphicDialog
{
public:
    /// Create new "insert graphics" dialog with given title
    SvxOpenGraphicDialog    ( const String& rTitle );
    ~SvxOpenGraphicDialog   ();

    /**
       Displays the dialog

       @return ERRCODE_NONE on success, ERRCODE_ABORT if user cancels, error of GraphicFilter::ImportGraphic() otherwise
    */
    ErrCode                 Execute();

    /// Set initially displayed path
    void                    SetPath( const String& rPath );
    /// Set initially displayed path and state of the "link" checkbox
    void                    SetPath( const String& rPath, sal_Bool bLinkState );
    /// Get displayed path
    String                  GetPath() const;

    /**
        Load selected graphic file (error if none selected)

        @return ERRCODE_NONE on success, error of GraphicFilter::ImportGraphic() otherwise
     */
    ErrCode                 GetGraphic(Graphic&) const;

    /// Enable/disable "link" checkbox
    void                    EnableLink(sal_Bool);
    /// Set displayed "link" checkbox state
    void                    AsLink(sal_Bool);
    /// Query displayed "link" checkbox state
    sal_Bool                IsAsLink() const;

    /// Query displayed filter string
    String                  GetCurrentFilter() const;
    /// Set displayed filter (chosen from the graphic filter list)
    void                    SetCurrentFilter(const String&);

private:
    // disable copy and assignment
    SvxOpenGraphicDialog    (const SvxOpenGraphicDialog&);
    SvxOpenGraphicDialog& operator = ( const SvxOpenGraphicDialog & );

    // safe pointer for impl class (no changes, automatic destruction)
    const std::auto_ptr< SvxOpenGrf_Impl >  mpImpl;
};

#endif // _SVX_OPENGRF_HXX

