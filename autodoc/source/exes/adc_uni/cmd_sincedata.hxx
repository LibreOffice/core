/*************************************************************************
 *
 *  $RCSfile: cmd_sincedata.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:36:02 $
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

#ifndef ADC_CMD_SINCEDATA_HXX
#define ADC_CMD_SINCEDATA_HXX



// USED SERVICES
    // BASE CLASSES
#include "adc_cmd.hxx"
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
namespace command
{


/** Provides replacements for the contents of the @@since documentation tag.

    Because the @@since tag is part of the source code, it allows only one kind
    of version information there. If this is to be mapped for different products
    (example: from OpenOffice.org versions in the @@since tag to StarOffice or
    StarSuite products), the value of @@since needs a replacement, which is provided
    by this class.

*/
class SinceTagTransformationData : public Context
{
  public:
    /** The key of this map are the version numbers within @since.
        The value is the string to display for each version number.
    */
    typedef std::map<String,String>     Map_Version2Display;

    // LIFECYCLE
                        SinceTagTransformationData();
    virtual             ~SinceTagTransformationData();

    // INQUIRY
    /// False, if no transformation table exists.
    bool                DoesTransform() const;

    /** Strips the value of an @@since tag down to the meaningful parts.

        The meaningful parts are considered
            - to start with the first cipher following a blank
            - to end with the end of the line
        and are
            - stripped of trailing blanks.
        If function returns false, io_sSinceTagValue stays unchanged.

        @return
        True, if a meaningful version number was found.
        A version number is meaningful, if it contains at least one cipher.
    */
    bool                StripSinceTagText(
                            String &            io_sSinceTagValue ) const;

    /** Gets the string to display for a version number.

        @param i_sVersionNumber
        Usually should be the result of ->StripSinceTagValue().
    */
    const String &      DisplayOf(
                            const String &      i_sVersionNumber ) const;
  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          i_nCurArgsBegin,
                            opt_iter            i_nEndOfAllArgs );
    // DATA
    Map_Version2Display aTransformationTable;
};


}   // namespace command
}   // namespace autodoc


#endif
