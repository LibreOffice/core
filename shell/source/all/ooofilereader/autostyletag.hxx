/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autostyletag.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 13:11:00 $
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


#ifndef AUTOSTYLETAG_HXX_INCLUDED
#define AUTOSTYLETAG_HXX_INCLUDED

#ifndef ITAG_HXX_INCLUDED
#include "itag.hxx"
#endif

/***************************   CAutoStyleTag tag readers   ***************************/

/** Implements the ITag interface for
    building a Style-Locale list

    Usage sample:

    LocaleSet_t locale = meta_info_accessor.getDefaultLocale();
    CContentReader content( m_szFileName, locale );
    CStyleMap map = content.getStyleMap();
*/
class CAutoStyleTag : public ITag
{
    public:
        CAutoStyleTag():m_CurrentStyleLocalePair( EMPTY_STYLELOCALE_PAIR ){};
        CAutoStyleTag( const XmlTagAttributes_t& attributes );

        virtual void startTag();
        virtual void endTag();
        virtual void addCharacters(const std::wstring& characters);
        virtual void addAttributes(const XmlTagAttributes_t& attributes);
        virtual std::wstring getTagContent(){ return EMPTY_STRING; };
        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & /*attrname*/ ){ return ::std::wstring() ; };

        void setStyle( ::std::wstring const & Style );
        void setLocale( LocaleSet_t Locale );
        void clearStyleLocalePair( void );
        inline StyleLocalePair_t getStyleLocalePair() const{ return m_CurrentStyleLocalePair; };
        inline bool isFull() const
        {
            return (( m_CurrentStyleLocalePair.first != EMPTY_STRING )&&
                   ( m_CurrentStyleLocalePair.second != EMPTY_LOCALE));
        };

    private:
        StyleLocalePair_t m_CurrentStyleLocalePair;
};

#endif