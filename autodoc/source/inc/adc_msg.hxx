/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adc_msg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:08:39 $
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

#ifndef ADC_ADC_MSG_HXX
#define ADC_ADC_MSG_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
namespace csv
{
    class File;
}


namespace autodoc
{


/** Gathers, sorts and displays (mainly diagnostic) messages to the
    user of Autodoc.
*/
class Messages
{
  public:
    // LIFECYCLE
                        Messages();
                        ~Messages();
    // OPERATIONS
    void                WriteFile(
                            const String &      i_sOutputFilePath);
    // INQUIRY

    // ACCESS
    void                Out_MissingDoc(
                            const String &      i_sEntity,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_ParseError(
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_InvalidConstSymbol(
                            const String &      i_sText,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_UnresolvedLink(
                            const String &      i_sLinkText,
                            const String &      i_sFile,
                            uintt               i_nLine);
    void                Out_TypeVsMemberMisuse(
                            const String &      i_sLinkText,
                            const String &      i_sFile,
                            uintt               i_nLine);

    static Messages &   The_();

  private:
    struct Location
    {
        String              sFile;
        uintt               nLine;

                            Location(
                                const String &      i_file,
                                uintt               i_line)
                                :   sFile(i_file),
                                    nLine(i_line)   {}
        bool                operator<(
                                const Location &    i_other) const
                                { int cmp = csv::compare(sFile,i_other.sFile);
                                  return cmp < 0
                                            ?   true
                                            :   cmp > 0
                                                    ?   false
                                                    :   nLine < i_other.nLine;
                                }
    };

    typedef std::map<Location,String>   MessageMap;

    // Locals
    void                AddValue(
                            MessageMap &        o_dest,
                            const String &      i_sText,
                            const String &      i_sFile,
                            uintt               i_nLine );
    void                WriteParagraph(
                            csv::File &         o_out,
                            const MessageMap &  i_source,
                            const String &      i_title,
                            const String &      i_firstIntermediateText );

    // DATA
    MessageMap          aMissingDocs;
    MessageMap          aParseErrors;
    MessageMap          aInvalidConstSymbols;
    MessageMap          aUnresolvedLinks;
    MessageMap          aTypeVsMemberMisuses;
};



// IMPLEMENTATION


}   // namespace autodoc

inline autodoc::Messages &
TheMessages()
{
    return autodoc::Messages::The_();
}

#endif
