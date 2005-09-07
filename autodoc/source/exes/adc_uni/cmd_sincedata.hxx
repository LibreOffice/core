/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cmd_sincedata.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:07:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
