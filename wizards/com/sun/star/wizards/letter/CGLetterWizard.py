from common.ConfigGroup import *
from CGLetter import CGLetter

class CGLetterWizard (ConfigGroup):

    def __init__(self):
        self.cp_LetterType = int()
        self.cp_BusinessLetter = CGLetter()
        self.cp_PrivateOfficialLetter = CGLetter()
        self.cp_PrivateLetter = CGLetter()
