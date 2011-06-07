'''
To change the template for this generated type comment go to
Window>Preferences>Java>Code Generation>Code and Comments
'''
class UIConsts():

    RID_COMMON = 500
    RID_DB_COMMON = 1000
    RID_FORM = 2200
    RID_QUERY = 2300
    RID_REPORT = 2400
    RID_TABLE = 2500
    RID_IMG_REPORT = 1000
    RID_IMG_FORM = 1100
    RID_IMG_WEB = 1200
    INVISIBLESTEP = 99
    INFOIMAGEURL = "private:resource/dbu/image/19205"
    INFOIMAGEURL_HC = "private:resource/dbu/image/19230"
    '''
    The tabindex of the navigation buttons in a wizard must be assigned a very
    high tabindex because on every step their taborder must appear at the end
    '''
    SOFIRSTWIZARDNAVITABINDEX = 30000
    INTEGER_8 = 8
    INTEGER_12 = 12
    INTEGER_14 = 14
    INTEGER_16 = 16
    INTEGER_40 = 40
    INTEGER_50 = 50

    #Steps of the QueryWizard

    SOFIELDSELECTIONPAGE = 1
    SOSORTINGPAGE = 2
    SOFILTERPAGE = 3
    SOAGGREGATEPAGE = 4
    SOGROUPSELECTIONPAGE = 5
    SOGROUPFILTERPAGE = 6
    SOTITLESPAGE = 7
    SOSUMMARYPAGE = 8
    INTEGERS = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    class CONTROLTYPE():

        BUTTON = 1
        IMAGECONTROL = 2
        LISTBOX = 3
        COMBOBOX = 4
        CHECKBOX = 5
        RADIOBUTTON = 6
        DATEFIELD = 7
        EDITCONTROL = 8
        FILECONTROL = 9
        FIXEDLINE = 10
        FIXEDTEXT = 11
        FORMATTEDFIELD = 12
        GROUPBOX = 13
        HYPERTEXT = 14
        NUMERICFIELD = 15
        PATTERNFIELD = 16
        PROGRESSBAR = 17
        ROADMAP = 18
        SCROLLBAR = 19
        TIMEFIELD = 20
        CURRENCYFIELD = 21
        UNKNOWN = -1
