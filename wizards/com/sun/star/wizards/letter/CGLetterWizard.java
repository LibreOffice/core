package com.sun.star.wizards.letter;

import com.sun.star.wizards.common.ConfigGroup;

public class CGLetterWizard extends ConfigGroup {

    public int cp_LetterType;
    public CGLetter cp_BusinessLetter = new CGLetter();
    public CGLetter cp_PrivateOfficialLetter = new CGLetter();
    public CGLetter cp_PrivateLetter = new CGLetter();

}
