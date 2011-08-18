from common.ConfigGroup import *
from CGFax import CGFax

class CGFaxWizard(ConfigGroup):

    def __init__(self):
        self.cp_FaxType = int()
        self.cp_BusinessFax = CGFax()
        self.cp_PrivateFax = CGFax()

