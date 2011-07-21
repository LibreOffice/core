from common.ConfigGroup import *
from common.ConfigSet import ConfigSet
from CGTopic import CGTopic

class CGAgenda(ConfigGroup):
    cp_AgendaType = int()
    cp_IncludeMinutes = bool()
    cp_Title = ""
    cp_Date = str()
    cp_Time = str()
    cp_Location = ""
    cp_ShowMeetingType = bool()
    cp_ShowRead = bool()
    cp_ShowBring = bool()
    cp_ShowNotes = bool()
    cp_ShowCalledBy = bool()
    cp_ShowFacilitator = bool()
    cp_ShowNotetaker = bool()
    cp_ShowTimekeeper = bool()
    cp_ShowAttendees = bool()
    cp_ShowObservers = bool()
    cp_ShowResourcePersons = bool()
    cp_TemplateName = str()
    cp_TemplatePath = str()
    cp_ProceedMethod = int()

    cp_Topics = ConfigSet(CGTopic())
