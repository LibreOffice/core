'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : place it here
'*
'\***********************************************************************
'*
' #1 fThesaurusLocales         'Get locales which are supported by Thesaurus
' #1 fSpellcheckerLocales      'Get locales which are supported by Spellchecker
'*
'\***********************************************************************

function fThesaurusLocales as boolean
'/// Gets the supported thesaurus locales from the API
'/// Returns TRUE if iSprache is supported by Thesaurus
'/// Returns FALSE if iSprache is not supported by Thesaurus

    dim uno as object
    dim linugServiceMgr as object
    dim aAllLocales (256) as variant
    dim sLocale as string
    dim i as integer
    
    fThesaurusLocales = FALSE
    uno=hGetUnoService()
    linugServiceMgr=uno.createInstance("com.sun.star.linguistic2.LinguServiceManager")
    aAllLocales = linugServiceMgr.getThesaurus().getLocales()
    for i = 0 to ubound( aAllLocales ())
        sLocale = ( aAllLocales(i).Language & "-" & (aAllLocales(i).Country) )
        printlog sLocale
        if ConvertLanguage2 ( sLocale ) = iSprache then
            fThesaurusLocales = TRUE
            i = ubound( aAllLocales ())
        endif
    next i

end function

'-----------------------------------------------------------

function fSpellcheckerLocales as boolean
'/// Gets the supported spellchecker locales from the API
'/// Returns TRUE if iSprache is supported by Spellchecker
'/// Returns FALSE if iSprache is not supported by Spellchecker

    dim uno as object
    dim linugServiceMgr as object
    dim aAllLocales (256) as variant
    dim sLocale as string
    dim i as integer
    
    fSpellcheckerLocales = FALSE
    uno=hGetUnoService()
    linugServiceMgr=uno.createInstance("com.sun.star.linguistic2.LinguServiceManager")
    aAllLocales = linugServiceMgr.getSpellchecker().getLocales()
    for i = 0 to ubound( aAllLocales ())
        sLocale = ( aAllLocales(i).Language & "-" & (aAllLocales(i).Country) )
        printlog sLocale
        if ConvertLanguage2 ( sLocale ) = iSprache then
            fSpellcheckerLocales = TRUE
            i = ubound( aAllLocales ())
        endif
    next i

end function

