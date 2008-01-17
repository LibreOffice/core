/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numberformatsbuffer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
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

#ifndef OOX_XLS_NUMBERFORMATSBUFFER_HXX
#define OOX_XLS_NUMBERFORMATSBUFFER_HXX

#include <com/sun/star/lang/Locale.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/stylespropertyhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { struct Locale; }
    namespace util { class XNumberFormats; }
} } }

namespace oox {
namespace xls {

// ============================================================================

struct OoxNumFmtData
{
    ::com::sun::star::lang::Locale maLocale;
    ::rtl::OUString     maFmtCode;
    sal_Int16           mnPredefId;

    explicit            OoxNumFmtData();
};

// ----------------------------------------------------------------------------

/** Contains all data for a number format code. */
class NumberFormat : public WorkbookHelper
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > XNumberFormatsRef;

public:
    explicit            NumberFormat( const WorkbookHelper& rHelper );

    /** Sets the passed format code. */
    void                setFormatCode( const ::rtl::OUString& rFmtCode );
    /** Sets the passed format code, encoded in UTF-8. */
    void                setFormatCode(
                            const ::com::sun::star::lang::Locale& rLocale,
                            const sal_Char* pcFmtCode );
    /** Sets the passed predefined format code identifier. */
    void                setPredefinedId(
                            const ::com::sun::star::lang::Locale& rLocale,
                            sal_Int16 nPredefId );

    /** Final processing after import of all style settings. */
    void                finalizeImport(
                            const XNumberFormatsRef& rxNumFmts,
                            const ::com::sun::star::lang::Locale& rFromLocale );

    /** Writes the number format to the passed property set. */
    void                writeToPropertySet( PropertySet& rPropSet ) const;

private:
    OoxNumFmtData       maOoxData;
    ApiNumFmtData       maApiData;
};

typedef ::boost::shared_ptr< NumberFormat > NumberFormatRef;

// ============================================================================

class NumberFormatsBuffer : public WorkbookHelper
{
public:
    explicit            NumberFormatsBuffer( const WorkbookHelper& rHelper );

    /** Inserts a new number format. */
    NumberFormatRef     createNumFmt( sal_Int32 nNumFmtId, const ::rtl::OUString& rFmtCode );

    /** Inserts a new number format code. */
    NumberFormatRef     importNumFmt( const AttributeList& rAttribs );
    /** Inserts a new number format code from a NUMFMT record. */
    void                importNumFmt( RecordInputStream& rStrm );
    /** Inserts a new number format code from a FORMAT record. */
    void                importFormat( BiffInputStream& rStrm );

    /** Final processing after import of all style settings. */
    void                finalizeImport();

    /** Writes the specified number format to the passed property set. */
    void                writeToPropertySet( PropertySet& rPropSet, sal_Int32 nNumFmtId ) const;

private:
    /** Inserts built-in number formats for the current system language. */
    void                insertBuiltinFormats();

private:
    typedef RefMap< sal_Int32, NumberFormat > NumberFormatMap;

    NumberFormatMap     maNumFmts;          /// List of number formats.
    ::rtl::OUString     maLocaleStr;        /// Current office locale.
    sal_Int32           mnNextBiffIndex;    /// Format id counter for BIFF2-BIFF4.
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

