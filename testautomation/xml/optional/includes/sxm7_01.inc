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
'/************************************************************************
'*
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : XML Math Include File
'*
'************************************************************************
'*
' #1 tunary_binary                   'Sample document from old office
' #1 trelations                      'Sample document from old office
' #1 tset_operations                 'Sample document from old office
' #1 tfunctions                      'Sample document from old office
' #1 toperators                      'Sample document from old office
' #1 tattributes                     'Sample document from old office
' #1 tothers                         'Sample document from old office
' #1 tbrackets                       'Sample document from old office
' #1 tformats                        'Sample document from old office
' #1 tall_commands                   'Sample document from old office
' #1 tall_commands_star_math_3       'Sample document from old office
' #1 tall_selections                 'Sample document from old office
' #1 tannuities                      'Sample document from old office
' #1 tcauchy                         'Sample document from old office
' #1 tchemicalelement                'Sample document from old office
' #1 tdefinitionstraightline         'Sample document from old office
' #1 teffectiveannualinterestrate    'Sample document from old office
' #1 thamiltonoperator               'Sample document from old office
' #1 tinvestmentreturns              'Sample document from old office
' #1 tlawoferrorpropagation          'Sample document from old office
' #1 tlimes                          'Sample document from old office
' #1 tlinearregression               'Sample document from old office
' #1 tmaxwell                        'Sample document from old office
' #1 tscalarproduct                  'Sample document from old office
' #1 tsinus                          'Sample document from old office
' #1 tsquareroot                     'Sample document from old office
' #1 tstandarddeviation              'Sample document from old office
' #1 tvectorproduct                  'Sample document from old office
'*
'\***********************************************************************

testcase tunary_binary
    printlog "+- unary-binary.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\1_unary-binary.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase trelations
    printlog "+- 2_relations.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\2_relations.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\2_relations.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tset_operations
    printlog "+- 3_set_operations.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\3_set_operations.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\1_unary-binary\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\3_set_operations.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tfunctions
    printlog "+- 4_functions.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\4_functions.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\4_functions.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase toperators
    printlog "+- 5_operators.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\5_operators.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\5_operators.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tattributes
    printlog "+- 6_attributes.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\6_attributes.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\6_attributes.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tothers
    printlog "+- 7_others.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\7_others.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\7_others.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\7_others.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\7_others") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\7_others\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\7_others\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\7_others\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\7_others.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tbrackets
    printlog "+- 8_brackets.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\8_brackets.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\8_brackets.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tformats
    printlog "+- 9_formats.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\9_formats.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\9_formats.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tall_commands
    printlog "+- all_commands.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\all_commands.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tall_commands_star_math_3
    printlog "+- all_commands_star_math_3.0.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\all_commands_star_math_3.0.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\all_commands_star_math_3.0.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tall_selections
    printlog "+- all_selections.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\all_selections.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\all_selections.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tannuities
    printlog "+- annuities.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\annuities.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\annuities.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\annuities.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\annuities") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\annuities\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\annuities\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\annuities\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\annuities.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tcauchy
    printlog "+- cauchy.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\cauchy.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\cauchy.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tchemicalelement
    printlog "+- chemicalelement.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\chemicalelement.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\chemicalelement.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tdefinitionstraightline
    printlog "+- definitionstraightline.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\definitionstraightline.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\definitionstraightline.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase teffectiveannualinterestrate
    printlog "+- effectiveannualinterestrate.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\effectiveannualinterestrate.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\effectiveannualinterestrate.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase thamiltonoperator
    printlog "+- hamiltonoperator.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\hamiltonoperator.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\hamiltonoperator.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tinvestmentreturns
    printlog "+- investmentreturns.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\investmentreturns.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\investmentreturns.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tlawoferrorpropagation
    printlog "+- lawoferrorpropagation.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\lawoferrorpropagation.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\lawoferrorpropagation.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tlimes
    printlog "+- limes.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\limes.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\limes.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\limes.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\limes") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\limes\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\limes\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\limes\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\limes.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tlinearregression
    printlog "+- linearregression.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\linearregression.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\linearregression.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tmaxwell
    printlog "+- maxwell.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\maxwell.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\maxwell.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tscalarproduct
    printlog "+- scalarproduct.smf"  
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\scalarproduct.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\scalarproduct.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tsinus
    printlog "+- sinus.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\sinus.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\sinus.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\sinus.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\sinus") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\sinus\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\sinus\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\sinus\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\sinus.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tsquareroot
    printlog "+- squareroot.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\squareroot.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\squareroot.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tstandarddeviation
    printlog "+- standarddeviation.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\standarddeviation.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\standarddeviation.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tvectorproduct
    printlog "+- vectorproduct.smf"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\math\so_binary\vectorproduct.smf"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct.sxm") , "StarOffice XML (Math)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct.sxm") , gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct\settings.xml")) = FALSE then
            warnlog "XML-file 'settings.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\math\level1\vectorproduct.sxm"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

