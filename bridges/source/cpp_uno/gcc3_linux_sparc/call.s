.global doFlushCode
doFlushCode:
.L:     flush %o0
        deccc %o1
        bne .L
        add %o0, 8, %o0
        retl
        nop
.size doFlushCode,(.-doFlushCode)
.align 8
