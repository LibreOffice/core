/*************************************************************************
 *
 *  $RCSfile: queries.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-04-04 13:28:32 $
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

#ifndef __FRAMEWORK_QUERIES_H_
#define __FRAMEWORK_QUERIES_H_

//_________________________________________________________________________________________________________________
//  own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_FILTERFLAGS_H_
#include <filterflags.h>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          These values describe our supported queries for type, filter ... properties.
                    They are used by our FilterFactory or ouer TypeDetection to return
                    subsets of our cached configuration.
*//*-*************************************************************************************************************/

#define FILTERQUERY_TEXTDOCUMENT                        DECLARE_ASCII("_filterquery_textdocument"                       )
#define FILTERQUERY_WEBDOCUMENT                         DECLARE_ASCII("_filterquery_webdocument"                        )
#define FILTERQUERY_GLOBALDOCUMENT                      DECLARE_ASCII("_filterquery_globaldocument"                     )
#define FILTERQUERY_CHARTDOCUMENT                       DECLARE_ASCII("_filterquery_chartdocument"                      )
#define FILTERQUERY_SPREADSHEETDOCUMENT                 DECLARE_ASCII("_filterquery_spreadsheetdocument"                )
#define FILTERQUERY_PRESENTATIONDOCUMENT                DECLARE_ASCII("_filterquery_presentationdocument"               )
#define FILTERQUERY_DRAWINGDOCUMENT                     DECLARE_ASCII("_filterquery_drawingdocument"                    )
#define FILTERQUERY_FORMULARPROPERTIES                  DECLARE_ASCII("_filterquery_formulaproperties"                  )

#define FILTERQUERY_TEXTDOCUMENT_WITHDEFAULT            DECLARE_ASCII("_filterquery_textdocument_withdefault"           )
#define FILTERQUERY_WEBDOCUMENT_WITHDEFAULT             DECLARE_ASCII("_filterquery_webdocument_withdefault"            )
#define FILTERQUERY_GLOBALDOCUMENT_WITHDEFAULT          DECLARE_ASCII("_filterquery_globaldocument_withdefault"         )
#define FILTERQUERY_CHARTDOCUMENT_WITHDEFAULT           DECLARE_ASCII("_filterquery_chartdocument_withdefault"          )
#define FILTERQUERY_SPREADSHEETDOCUMENT_WITHDEFAULT     DECLARE_ASCII("_filterquery_spreadsheetdocument_withdefault"    )
#define FILTERQUERY_PRESENTATIONDOCUMENT_WITHDEFAULT    DECLARE_ASCII("_filterquery_presentationdocument_withdefault"   )
#define FILTERQUERY_DRAWINGDOCUMENT_WITHDEFAULT         DECLARE_ASCII("_filterquery_drawingdocument_withdefault"        )
#define FILTERQUERY_FORMULARPROPERTIES_WITHDEFAULT      DECLARE_ASCII("_filterquery_formulaproperties_withdefault"      )

#define FILTERQUERY_DEFAULTFILTER                       DECLARE_ASCII("_filterquery_defaultfilter"                      )

/*-************************************************************************************************************//**
    @short          These values describe properties which represent all items of a query.
*//*-*************************************************************************************************************/

// compare it with string Filter.DocumentService
#define DETECTPROPERTY_TEXTDOCUMENT                     DECLARE_ASCII("com.sun.star.text.TextDocument"                  )
#define DETECTPROPERTY_WEBDOCUMENT                      DECLARE_ASCII("com.sun.star.text.WebDocument"                   )
#define DETECTPROPERTY_GLOBALDOCUMENT                   DECLARE_ASCII("com.sun.star.text.GlobalDocument"                )
#define DETECTPROPERTY_CHARTDOCUMENT                    DECLARE_ASCII("com.sun.star.chart.ChartDocument"                )
#define DETECTPROPERTY_SPREADSHEETDOCUMENT              DECLARE_ASCII("com.sun.star.sheet.SpreadsheetDocument"          )
#define DETECTPROPERTY_PRESENTATIONDOCUMENT             DECLARE_ASCII("com.sun.star.presentation.PresentationDocument"  )
#define DETECTPROPERTY_DRAWINGDOCUMENT                  DECLARE_ASCII("com.sun.star.drawing.DrawingDocument"            )
#define DETECTPROPERTY_FORMULARPROPERTIES               DECLARE_ASCII("com.sun.star.formula.FormulaProperties"          )

// compare it with string Filter.DocumentService
#define DETECTPROPERTY_DEFAULTFILTER                    FILTERFLAG_DEFAULT

//_________________________________________________________________________________________________________________
//  methods
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          This method checks if given string match to a supported query.
*//*-*************************************************************************************************************/

inline sal_Bool c_isQuery( const ::rtl::OUString& sQuery )
{
    return  (
                ( sQuery == FILTERQUERY_TEXTDOCUMENT                        )   ||
                ( sQuery == FILTERQUERY_WEBDOCUMENT                         )   ||
                ( sQuery == FILTERQUERY_GLOBALDOCUMENT                      )   ||
                ( sQuery == FILTERQUERY_CHARTDOCUMENT                       )   ||
                ( sQuery == FILTERQUERY_SPREADSHEETDOCUMENT                 )   ||
                ( sQuery == FILTERQUERY_PRESENTATIONDOCUMENT                )   ||
                ( sQuery == FILTERQUERY_DRAWINGDOCUMENT                     )   ||
                ( sQuery == FILTERQUERY_FORMULARPROPERTIES                  )   ||
                ( sQuery == FILTERQUERY_TEXTDOCUMENT_WITHDEFAULT            )   ||
                ( sQuery == FILTERQUERY_WEBDOCUMENT_WITHDEFAULT             )   ||
                ( sQuery == FILTERQUERY_GLOBALDOCUMENT_WITHDEFAULT          )   ||
                ( sQuery == FILTERQUERY_CHARTDOCUMENT_WITHDEFAULT           )   ||
                ( sQuery == FILTERQUERY_SPREADSHEETDOCUMENT_WITHDEFAULT     )   ||
                ( sQuery == FILTERQUERY_PRESENTATIONDOCUMENT_WITHDEFAULT    )   ||
                ( sQuery == FILTERQUERY_DRAWINGDOCUMENT_WITHDEFAULT         )   ||
                ( sQuery == FILTERQUERY_FORMULARPROPERTIES_WITHDEFAULT      )   ||
                ( sQuery == FILTERQUERY_DEFAULTFILTER                       )
            );
}

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_QUERIES_H_
