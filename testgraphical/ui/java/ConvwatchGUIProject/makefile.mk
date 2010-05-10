

dist/ConvwatchGUIProject.jar: src/*.java
.if $(JDK_VERSION) < 160
    echo "You need at least java 6"
    error
.endif
    ant

clean:
    ant clean
