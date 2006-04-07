#!/usr/bin/awk -f
#
# Utility to compare MS-LANGID definitions with those defined in ../../inc/i18npool/lang.hxx
# Run in i18npool/source/isolang
#
# outputs new #define LANGUAGE_... 0x... and also some commented out substrings
# that were matched in already existing defines.
#
# Expects input from the saved page of
#
# (1)
# http://www.microsoft.com/globaldev/reference/lcid-all.mspx
# filtered through ``html2text -nobs ...'', generated table:
# blank,name,hex,dec fields:
#    |Afrikaans_-_South_Africa____________|0436|1078_|
# Best if file cleaned up to _only_ contain the table entries, but not
# necessary, entries are filtered. Check output.
#
# complete command line:
# lynx -dump -source http://www.microsoft.com/globaldev/reference/lcid-all.mspx | html2text -nobs | awk -f lcid.awk >outfile
#
#
# (2)
# http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx
# filtered through ``html2text -nobs ...'', generated table:
# blank,name,hex,dec,inputlocales,collection fields:
#    |Afrikaans                           |0436|1078 |0436:       |Basic     |
# Best if file cleaned up to _only_ contain the table entries, but not
# necessary, entries are filtered. Check output.
#
# complete command line:
# lynx -dump -source http://www.microsoft.com/globaldev/reference/winxp/xp-lcid.mspx | html2text -nobs | awk -f lcid.awk >outfile
#
# Author: Eike Rathke <erack@sun.com>, <er@openoffice.org>
#

BEGIN {
    while ((getline < "../../inc/i18npool/lang.hxx") > 0)
    {
        if ($0 ~ /^#define[ ]*LANGUAGE_[_A-Za-z0-9]*[ ]*0x[0-9a-fA-F]/)
        {
            # lang[HEX]=NAME 
            lang[toupper(substr($3,3))] = toupper($2)
            #print substr($3,3) "=" $2
        }
    }
    # html2text table follows
    FS = "\|"
    filetype = 0
    lcid_all = 1
    xp_lcid  = 2
}

(NF < 5) { next }

!filetype {
    if (NF == 5)
        filetype = lcid_all
    else if (NF == 7)
        filetype = xp_lcid
    else
        next
}

($3 !~ /^[0-9a-fA-F][0-9a-fA-F]*$/) { filtered[$3] = $0; next }

# all[HEX]=string
{ all[toupper($3)] = $2 }

# new hex: newlang[HEX]=string
!(toupper($3) in lang) { newlang[toupper($3)] = $2 }

END {
    if (!filetype)
    {
        print "No file type (lcid-all|xp-lcid) recognized." >>"/dev/stderr"
        exit(1)
    }
    else if (filetype == lcid_all)
        print "// assuming lcid-all file"
    else if (filetype == xp_lcid)
        print "// assuming xp-lcid file"
    else
        print "// unknown file type"
    # every new language
    for (x in newlang)
    {
        printf( "xxxxxxx LANGUAGE_%-26s 0x%s\n", newlang[x], x)
        n = split(newlang[x],arr,/[^A-Za-z0-9]/)
        def = ""
        for (i=1; i<=n; ++i)
        {
            if (length(arr[i]))
            {
                # each identifier word of the language name
                if (def)
                    def = def "_"
                aup = toupper(arr[i])
                def = def aup
                for (l in lang)
                {
                    #  contained in already existing definitions?
                    if (lang[l] ~ aup)
                        printf( "// %-50s %s\n", arr[i] ": " lang[l], l)
                }
            }
        }
        printf( "#define LANGUAGE_%-26s 0x%s\n", def, x)
    }
    print "\n// --- reverse check follows ----------------------------------\n"
    for (x in lang)
    {
        if (!(x in all))
            print "// not in input file:   " x "  " lang[x]
    }
    print "\n// --- filtered table entries follow (if any) -----------------\n"
    for (x in filtered)
        print "// filtered:   " x "  " filtered[x]
}
