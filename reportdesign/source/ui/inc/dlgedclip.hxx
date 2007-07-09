#ifndef _REPORT_RPTUICLIP_HXX
#define _REPORT_RPTUICLIP_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgedclip.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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


#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

namespace rptui
{
//============================================================================
// OReportExchange
//============================================================================
/** \class OReportExchange
 * \brief defines a clipboard fromat for copying selection elements.
 * \ingroup reportdesign_source_ui_report
 *
 *
 */
class OReportExchange : public TransferableHelper
{
public:
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > TSectionElements;

    /** Constructs a new exchange object with section elements
    *
    * \param _rCopyElements the elements to copy. Each section is one entry. The value must be a sequence of elements.
    * \return
    */
    OReportExchange( const TSectionElements& _rCopyElements);

    /** checks whether or not a descriptor can be extracted from the data flavor vector given
    *
    * \param _rFlavors
            available flavors
    * \return
    */
    static sal_Bool         canExtract(const DataFlavorExVector& _rFlavors);

    /** extract the section elements
    *
    * \param _rData the clipboard data
    * \return the copied elements
    */
    static TSectionElements extractCopies(const TransferableDataHelper& _rData);

    /** returns the format id.
    *
    * \return the registered format id
    */
    static sal_uInt32       getDescriptorFormatId();

protected:
    // TransferableHelper overridables
    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
private:
    TSectionElements    m_aCopyElements;
};
}
#endif // _REPORT_RPTUICLIP_HXX
