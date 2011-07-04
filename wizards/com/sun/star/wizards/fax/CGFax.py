from common.ConfigGroup import *

class CGFax(ConfigGroup):

    def __init__(self):

        self.cp_Style = int()
        self.cp_PrintCompanyLogo = bool()
        self.cp_PrintDate = bool()
        self.cp_PrintSubjectLine = bool()
        self.cp_PrintSalutation = bool()
        self.cp_PrintCommunicationType = bool()
        self.cp_PrintGreeting = bool()
        self.cp_PrintFooter = bool()
        self.cp_CommunicationType = str()
        self.cp_Salutation = str()
        self.cp_Greeting = str()
        self.cp_SenderAddressType = int()
        self.cp_SenderCompanyName = str()
        self.cp_SenderStreet = str()
        self.cp_SenderPostCode = str()
        self.cp_SenderState = str()
        self.cp_SenderCity = str()
        self.cp_SenderFax = str()
        self.cp_ReceiverAddressType = int()
        self.cp_Footer = str()
        self.cp_FooterOnlySecondPage = bool()
        self.cp_FooterPageNumbers = bool()
        self.cp_CreationType = int()
        self.cp_TemplateName = str()
        self.cp_TemplatePath = str()
